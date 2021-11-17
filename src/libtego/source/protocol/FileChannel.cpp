/* Ricochet Refresh - https://ricochetrefresh.net/
 * Copyright (C) 2020, Blueprint For Free Speech <ricochet@blueprintforfreespeech.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *
 *    * Neither the names of the copyright owners nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "FileChannel.h"
#include "Channel_p.h"
#include "Connection.h"
#include "utils/SecureRNG.h"
#include "utils/Useful.h"

#include "context.hpp"
#include "error.hpp"
#include "globals.hpp"
#include "file_hash.hpp"
using tego::g_globals;

using namespace Protocol;

static void logTransferStats(qint64 bytes, std::chrono::time_point<std::chrono::system_clock> beginTime)
{
    // This is preferred over `static_cast<double>(bytes) / 1024.0` because
    // doing it that way will potentially lose precision if the byte count is
    // above 9007199254740992, which while albeit unlikely, is theoretically 
    // possible.
    const auto kilobytes = static_cast<double>(bytes / 1024) + (static_cast<double>(bytes % 1024) / 1024);
    const auto seconds = std::chrono::duration_cast<std::chrono::duration<double>>( std::chrono::system_clock::now() - beginTime).count();

    logger::println("Transfer Complete: {{ size : {} kilobytes, duration : {} seconds, rate : {} kilobytes / second}}", kilobytes, seconds, kilobytes / seconds);
}

//
// Outgoing Transfer Record
//

FileChannel::outgoing_transfer_record::outgoing_transfer_record(
    tego_file_transfer_id_t transferId,
    const std::string& filePath,
    tego_file_size_t fileSize)
: id(transferId)
, size(fileSize)
, offset(0)
, stream(filePath, std::ios::in | std::ios::binary)
{ }

//
// Incoming Transfer Record
//

FileChannel::incoming_transfer_record::incoming_transfer_record(
    tego_file_transfer_id_t transferId,
    tego_file_size_t fileSize,
    const std::string& fileHash)
: id(transferId)
, size(fileSize)
, hash(fileHash)
, stream()
{ }

FileChannel::incoming_transfer_record::~incoming_transfer_record()
{
    if (this->stream.is_open())
    {
        // try our best to remove the partial file
        this->stream.close();

        // ignore error here, if incoming request succeeded then the
        // partial should no longer exist
        QFile::remove(QString::fromStdString(this->partial_dest()));
    }
}

std::string FileChannel::incoming_transfer_record::partial_dest() const
{
    return dest + ".part";
}

void FileChannel::incoming_transfer_record::open_stream(const std::string& destination)
{
    this->dest = destination;

    // attempt to open the destination for reading and writing
    // discard previous contents
    // binary mode
    // we need to read to validate the hash after the transfer completes
    this->stream.open(this->partial_dest(), std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
    TEGO_THROW_IF_FALSE(this->stream.is_open());
}

//
// File Channel
//

FileChannel::FileChannel(Direction direction, Connection *connection)
    : Channel(QStringLiteral("im.ricochet.file-transfer"), direction, connection)
{
    connect(this->d_ptr->connection, &Connection::closed, this, &FileChannel::onConnectionClosed);
}

bool FileChannel::allowInboundChannelRequest(
    const Data::Control::OpenChannel*,
    Data::Control::ChannelResult *result)
{
    if (connection()->purpose() != Connection::Purpose::KnownContact) {
        qDebug() << "Rejecting request for" << type() << "channel from connection with purpose" << int(connection()->purpose());
        result->set_common_error(Data::Control::ChannelResult::UnauthorizedError);
        return false;
    }

    if (connection()->findChannel<FileChannel>(Channel::Inbound)) {
        qDebug() << "Rejecting request for" << type() << "channel because one is already open";
        return false;
    }

    return true;
}

bool FileChannel::allowOutboundChannelRequest(
    Data::Control::OpenChannel*)
{
    if (connection()->findChannel<FileChannel>(Channel::Outbound)) {
        TEGO_BUG() << "Rejecting outbound request for" << type() << "channel because one is already open on this connection";
        return false;
    }

    if (connection()->purpose() != Connection::Purpose::KnownContact) {
        TEGO_BUG() << "Rejecting outbound request for" << type() << "channel for connection with unexpected purpose" << int(connection()->purpose());
        return false;
    }

    return true;
}

bool FileChannel::verifyPacket(Data::File::Packet const& message)
{
    // ensure the packet has only 1 of the possible file messages

    auto messageCount = 0;
    messageCount += message.has_file_header();
    messageCount += message.has_file_header_ack();
    messageCount += message.has_file_chunk();
    messageCount += message.has_file_header_response();
    messageCount += message.has_file_chunk_ack();
    messageCount += message.has_file_transfer_complete_notification();

    if (messageCount == 1)
    {
        if (message.has_file_header()) {
            return verifyFileHeader(message.file_header());
        } else if (message.has_file_header_ack()) {
            return verifyFileHeaderAck(message.file_header_ack());
        } else if (message.has_file_chunk()) {
            return verifyFileChunk(message.file_chunk());
        } else if (message.has_file_header_response()) {
            return verifyFileHeaderResponse(message.file_header_response());
        } else if (message.has_file_chunk_ack()) {
            return verifyFileChunkAck(message.file_chunk_ack());
        } else if (message.has_file_transfer_complete_notification()) {
            return verifyFileTransferCompleteNotification(message.file_transfer_complete_notification());
        }
    }

    return false;
}

bool FileChannel::verifyFileHeader(Data::File::FileHeader const& message)
{
    return message.has_file_id() &&
           message.has_file_size() &&
           message.has_name() &&
           message.has_file_hash();
}

bool FileChannel::verifyFileHeaderAck(Data::File::FileHeaderAck const& message)
{
    return message.has_file_id() && message.has_accepted();
}

bool FileChannel::verifyFileHeaderResponse(Data::File::FileHeaderResponse const& message)
{
    return message.has_file_id() && message.has_response();
}

bool FileChannel::verifyFileChunk(Data::File::FileChunk const& message)
{
    return message.has_file_id() && message.has_chunk_data();
}

bool FileChannel::verifyFileChunkAck(Data::File::FileChunkAck const& message)
{
    return message.has_file_id() && message.has_bytes_received();
}

bool FileChannel::verifyFileTransferCompleteNotification(Data::File::FileTransferCompleteNotification const& message)
{
    return message.has_file_id() && message.has_result();
}

void FileChannel::receivePacket(const QByteArray &packet)
{
    Data::File::Packet message;
    if (!message.ParseFromArray(packet.constData(), packet.size())) {
        emitFatalError("Failed to parse message on file channel", tego_file_transfer_result_failure, true);
        return;
    }

    if (!verifyPacket(message))
    {
        emitFatalError("Failed to verify message on file channel", tego_file_transfer_result_failure, true);
        return;
    }

    if (message.has_file_header()) {
        handleFileHeader(message.file_header());
    } else if (message.has_file_header_ack()) {
        handleFileHeaderAck(message.file_header_ack());
    } else if (message.has_file_chunk()) {
        handleFileChunk(message.file_chunk());
    } else if (message.has_file_header_response()) {
        handleFileHeaderResponse(message.file_header_response());
    } else if (message.has_file_chunk_ack()) {
        handleFileChunkAck(message.file_chunk_ack());
    } else if (message.has_file_transfer_complete_notification()) {
        handleFileTransferCompleteNotification(message.file_transfer_complete_notification());
    } else {
        emitFatalError("Unrecognized file packet on FileChannel", tego_file_transfer_result_failure, true);
    }
}

void FileChannel::onConnectionClosed()
{
    // we do not need to close the channel here because our owning Connection
    // will already do so, from ConnectionPrivate::socketDisconnected
    this->emitFatalError("Connection Closed", tego_file_transfer_result_network_error, false);
}

//
// Error Handling
//

void FileChannel::emitFatalError(std::string&& message, tego_file_transfer_result_t error, bool shouldCloseChannel)
{
    qWarning() << message.data();

    // tear down all ongoing transfers
    switch(direction())
    {
    case Inbound:
        for(const auto& [id, itr] : incomingTransfers)
        {
            emit this->fileTransferFinished(id, tego_file_transfer_direction_receiving, error);
        }
        incomingTransfers.clear();
        break;
    case Outbound:
        for(const auto& [id, itr] : outgoingTransfers)
        {
            emit this->fileTransferFinished(id, tego_file_transfer_direction_sending, error);
        }
        outgoingTransfers.clear();
        break;
    default:
        break;
    }

    if (shouldCloseChannel)
    {
        qWarning() << "Closing Channel";
        this->closeChannel();
    }
}

void FileChannel::emitNonFatalError(std::string&& message, tego_file_transfer_id_t id, tego_file_transfer_result_t error)
{
    // log error message to console
    qWarning() << message.data();

    const auto direction = this->direction();
    switch(direction)
    {
    case Inbound:
        if (auto it = incomingTransfers.find(id); it != incomingTransfers.end())
        {
            emit this->fileTransferFinished(id, tego_file_transfer_direction_receiving, error);
            incomingTransfers.erase(it);
        }
        break;
    case Outbound:
        if (auto it = outgoingTransfers.find(id); it != outgoingTransfers.end())
        {
            emit this->fileTransferFinished(id, tego_file_transfer_direction_sending, error);
            outgoingTransfers.erase(it);
        }
        break;
    default:
        emitFatalError(fmt::format("Unknown FileChannel::direction()", direction), tego_file_transfer_result_failure, true);
        break;
    }
}

// verify that all the file_id members are the right size
template<typename S>
constexpr static bool has_compatible_file_id()
{
    typedef decltype(S().file_id()) file_id_t;
    return std::numeric_limits<tego_file_transfer_id_t>::max() == std::numeric_limits<file_id_t>::max() &&
           std::numeric_limits<tego_file_transfer_id_t>::min() == std::numeric_limits<file_id_t>::min() &&
           std::is_signed_v<tego_file_transfer_id_t> == std::is_signed_v<file_id_t>;
}

static_assert(has_compatible_file_id<Data::File::FileHeader>());
static_assert(has_compatible_file_id<Data::File::FileHeaderAck>());
static_assert(has_compatible_file_id<Data::File::FileHeaderResponse>());
static_assert(has_compatible_file_id<Data::File::FileChunk>());
static_assert(has_compatible_file_id<Data::File::FileChunkAck>());
static_assert(has_compatible_file_id<Data::File::FileTransferCompleteNotification>());


void FileChannel::handleFileHeader(const Data::File::FileHeader &message)
{
    Q_ASSERT(direction() == Inbound);

    auto response = std::make_unique<Data::File::FileHeaderAck>();
    response->set_accepted(false);

    if (message.name().find("..") != std::string::npos)
    {
        qWarning() << "Rejected file header with name containing '..'";
    }
    else if (message.name().find("/") != std::string::npos)
    {
        qWarning() << "Rejected file header with name containing '/'";
    }
    // ensure the hash is the correct length
    else if (message.file_hash().size() != tego_file_hash::DIGEST_SIZE)
    {
        qWarning() << "Rejected file header with hash incorrect length";
    }
    else
    {
        // ensure that we can write a file this large
        if constexpr(std::numeric_limits<qint64>::max() > std::numeric_limits<std::streamoff>::max())
        {
            if(message.file_size() > std::numeric_limits<std::streamoff>::max())
            {
                qWarning() << "Rejected file header with too large a file size";
            }
        }

        tego_file_hash fileHash;
        const auto& digest = message.file_hash();
        // copy our digest in directly
        std::copy(digest.begin(), digest.end(), fileHash.data.begin());

        const auto id = message.file_id();
        incoming_transfer_record ifr(id, message.file_size(), fileHash.to_string());

        // signal the file transfer request
        emit this->fileTransferRequestReceived(id, QString::fromStdString(message.name()), ifr.size, std::move(fileHash));

        incomingTransfers.insert({id, std::move(ifr)});

        response->set_file_id(id);
        response->set_accepted(true);
    }

    // finally send our ack for the header
    Data::File::Packet packet;
    packet.set_allocated_file_header_ack(response.release());
    Channel::sendMessage(packet);
}

void FileChannel::handleFileHeaderAck(const Data::File::FileHeaderAck &message)
{
    if (direction() != Outbound) {
        qWarning() << "Rejected inbound acknowledgement on an inbound file channel";
        closeChannel();
        return;
    }

    auto id = message.file_id();
    if (outgoingTransfers.contains(id))
    {
        emit this->fileTransferAcknowledged(id, message.accepted());
    } else {
        qDebug() << "Received file acknowledgement for unknown message" << id;
    }
}

void FileChannel::handleFileHeaderResponse(const Data::File::FileHeaderResponse &message)
{
    if (direction() != Outbound) {
        emitFatalError("Rejected FileHeaderResponse message on inbound file channel", tego_file_transfer_result_failure, true);
        return;
    }

    const auto id = message.file_id();

    auto it = outgoingTransfers.find(id);
    if (it == outgoingTransfers.end())
    {
        // this can happen if local user cancels a transfer request before their receiver has responded,
        // so this is not a fatal error
        qWarning() << "recieved response for a file header we never sent";
        return;
    }

    const auto response = message.response();
    emit this->fileTransferRequestResponded(message.file_id(), static_cast<tego_file_transfer_response_t>(response));

    if (response == tego_file_transfer_response_accept)
    {
        sendNextChunk(id);
        it->second.beginTime = std::chrono::system_clock::now();
    }
    else
    {
        if (response != tego_file_transfer_response_reject)
        {
            // this can never happen kill connection if we receive invalid value here
            emitFatalError("Received invalid FileHeaderResponse", tego_file_transfer_result_failure, true);
            return;
        }
        // receiver rejected our transfer request, so erase it from our records
        outgoingTransfers.erase(it);
    }
}

void FileChannel::handleFileChunk(const Data::File::FileChunk &message)
{
    if (direction() != Inbound)
    {
        emitFatalError("Rejected FileChunk message on outbound file channel", tego_file_transfer_result_failure, true);
        return;
    }

    auto it = incomingTransfers.find(message.file_id());
    if (it == incomingTransfers.end())
    {
        // we can receive an unknown chunk if we cancel in the middle of transmission
        // so this is fine
        qWarning() << "rejecting chunk for unknown file";
        return;
    }
    else if (message.chunk_data().size() > FileMaxChunkSize)
    {
        // something is very wrong in this case
        emitFatalError("Rejected FileChunk because of invalid chunk_data() size", tego_file_transfer_result_failure, true);
        return;
    }
    else
    {
        auto& itr = it->second;
        const auto& chunk_data = message.chunk_data();
        itr.stream.write(chunk_data.data(), static_cast<std::streamsize>(chunk_data.size()));

        // emit progress callback
        const auto id = message.file_id();
        const auto streamOffset = static_cast<std::streamoff>(itr.stream.tellg());
        if (streamOffset == std::streamoff(-1))
        {
            // we should send complete message to sender if we have a disk error so they do not spam us with chunks
            // we can't do anything with; this transfer is not recoverable, but others can continue
            emitNonFatalError("Error writing chunk to stream", id, tego_file_transfer_result_filesystem_error);

            // send message to transfer partner to let them know we've given up
            auto notification = std::make_unique<Data::File::FileTransferCompleteNotification>();
            notification->set_file_id(id);
            notification->set_result(Protocol::Data::File::Cancelled);

            Data::File::Packet packet;
            packet.set_allocated_file_transfer_complete_notification(notification.release());
            Channel::sendMessage(packet);

            return;
        }

        const auto bytesWritten = static_cast<tego_file_size_t>(streamOffset);
        const auto& bytesTotal = itr.size;

        emit this->fileTransferProgress(id, tego_file_transfer_direction_receiving, bytesWritten, bytesTotal);

        auto response = std::make_unique<Data::File::FileChunkAck>();
        response->set_file_id(message.file_id());
        response->set_bytes_received(bytesWritten);

        Data::File::Packet ackPacket;
        ackPacket.set_allocated_file_chunk_ack(response.release());
        Channel::sendMessage(ackPacket);

        if (bytesWritten == bytesTotal)
        {
            // reset the read/write stream and calculate the file hash
            itr.stream.seekg(0);
            tego_file_hash fileHash(itr.stream);
            itr.stream.close();

            if (fileHash.to_string() != itr.hash)
            {
                // delete file if calculated hash doesn't match expected
                QFile::remove(QString::fromStdString(itr.partial_dest()));
                emit this->fileTransferFinished(id, tego_file_transfer_direction_receiving, tego_file_transfer_result_bad_hash);
            }
            else
            {
                // if a file already exists at our final destination, then remove it
                const auto qDest = QString::fromStdString(itr.dest);
                if (QFile::exists(qDest))
                {
                    QFile::remove(qDest);
                }

                // move our partial file to final destination
                const auto qPartialDest = QString::fromStdString(itr.partial_dest());
                if(QFile::rename(qPartialDest, qDest))
                {
                    emit this->fileTransferFinished(id, tego_file_transfer_direction_receiving, tego_file_transfer_result_success);
                    logTransferStats(static_cast<qint64>(itr.size), itr.beginTime);
                }
                else
                {
                    emit this->fileTransferFinished(id, tego_file_transfer_direction_receiving, tego_file_transfer_result_filesystem_error);
                }
            }
            incomingTransfers.erase(it);

            // send complete notification to remote user
            auto notification = std::make_unique<Data::File::FileTransferCompleteNotification>();
            notification->set_file_id(id);
            notification->set_result(Protocol::Data::File::Success);

            Data::File::Packet notifPacket;
            notifPacket.set_allocated_file_transfer_complete_notification(notification.release());
            Channel::sendMessage(notifPacket);
        }
    }
}

void FileChannel::handleFileChunkAck(const Data::File::FileChunkAck &message)
{
    if (direction() != Outbound)
    {
        emitFatalError("Rejected FileChunkAck message on incoming file channel", tego_file_transfer_result_failure, true);
        return;
    }

    const auto id = message.file_id();

    auto it = outgoingTransfers.find(id);
    if (it == outgoingTransfers.end())
    {
        // we can get here if the sender cancels a transfer and an ack comes in from previously sent chunk
        // not an error
        qWarning() << "recieved ack for a chunk we never sent";
        return;
    }

    const auto& otr = it->second;

    // verify the ack corresponds to how many bytes we've sent
    if (message.bytes_received() != otr.offset)
    {
        // acks currently always come between sending chunks, so our bytes sent and their bytes received should
        // not diverge
        emitFatalError("mismatch between bytes we have sent and the bytes the receiver claims to have received", tego_file_transfer_result_failure, true);
        return;
    }

    emit this->fileTransferProgress(otr.id, tego_file_transfer_direction_receiving, otr.offset, otr.size);

    // send the next chunk until we are done
    if(otr.offset < otr.size)
    {
        sendNextChunk(id);
    }
}

// statically verify that our tego_file_transfer_result_t enum matches the FileTransferResult enum
typedef int file_transfer_result_underlying_t;
constexpr bool operator==(Protocol::Data::File::FileTransferResult left, tego_file_transfer_result_t right)
{
    return static_cast<file_transfer_result_underlying_t>(left) == static_cast<file_transfer_result_underlying_t>(right);
}

constexpr bool operator==(tego_file_transfer_result_t left, Protocol::Data::File::FileTransferResult right)
{
    return right == left;
}

static_assert(Protocol::Data::File::Success == tego_file_transfer_result_success);
static_assert(Protocol::Data::File::Cancelled == tego_file_transfer_result_cancelled);
static_assert(Protocol::Data::File::Failure == tego_file_transfer_result_failure);

void FileChannel::handleFileTransferCompleteNotification(const Data::File::FileTransferCompleteNotification &message)
{
    const auto id = message.file_id();

    Q_ASSERT(direction() == Outbound || direction() == Inbound);

    switch(direction())
    {
    case Inbound:
        if( auto it = incomingTransfers.find(id); it != incomingTransfers.end())
        {
            incomingTransfers.erase(it);
            emit fileTransferFinished(id, tego_file_transfer_direction_receiving, static_cast<tego_file_transfer_result_t>(message.result()));
            return;
        }
        break;
    case Outbound:
        if (auto it = outgoingTransfers.find(id); it != outgoingTransfers.end())
        {
            const auto& otr = it->second;
            if (message.result() == tego_file_transfer_result_success)
            {
                logTransferStats(static_cast<qint64>(otr.size), otr.beginTime);
            }

            outgoingTransfers.erase(it);
            emit fileTransferFinished(id, tego_file_transfer_direction_sending, static_cast<tego_file_transfer_result_t>(message.result()));
            return;
        }
        break;
    default:
        break;
    }
    // we could get here if we cancel a transfer locally before the last chunk has been ack'd, so not an error
    qWarning() << "received cancel request for unknown transfer:" << id;
}

bool FileChannel::sendFileWithId(QString file_uri,
                                 tego_file_hash_t const& file_hash,
                                 QDateTime,
                                 tego_file_transfer_id_t file_id)
{
    Q_ASSERT(direction() == Outbound);
    Q_ASSERT(!outgoingTransfers.contains(file_id));

    // verify the args
    Q_ASSERT(!file_uri.isEmpty());

    /* only allow regular files or symlinks chains to regular files */
    QFileInfo fi(file_uri);
    if (!fi.exists())
    {
        // this error state is bubbled up to ConversationModel
        qWarning() << "File does not exist";
        return false;
    }

    // canonical file path must not be empty if the file exists
    const auto canonicalFilePath = fi.canonicalFilePath();
    Q_ASSERT(!canonicalFilePath.isEmpty());

    // file size must be positive, QFileInfo::size() returns signed 64 bit int, so so long as
    // we are positive we'll fit into a tego_file_size_t which is a 64 bit unsigned int
    Q_ASSERT(fi.size() > 0);

    // ensure this file's size can be represented as a std::streamoff (the integer type of our offset into a std::ofstream)
    // if std::streamoff is a smaller type than qint64 (we only need to do this if streamoff is smaller than qint64)
    if constexpr(std::numeric_limits<std::streamoff>::max() < std::numeric_limits<qint64>::max())
    {
        TEGO_THROW_IF_FALSE(fi.size() <= std::numeric_limits<std::streamoff>::max());
    }

    const auto fileSize = static_cast<tego_file_size_t>(fi.size());

    // create our record
    const auto filePath = canonicalFilePath.toStdString();
    outgoing_transfer_record otr(file_id, filePath, fileSize);
    if (!otr.stream.is_open())
    {
        qWarning() << "Failed to open file for sending header";
        // this error state is bubbled up to ConversationModel
        return false;
    }
    outgoingTransfers.insert({file_id, std::move(otr)});

    // send file header to recipient
    auto header = std::make_unique<Data::File::FileHeader>();
    header->set_file_id(file_id);
    header->set_file_size(fileSize);
    header->set_file_hash(file_hash.data.data(), file_hash.data.size());
    header->set_name(fi.fileName().toStdString());

    Data::File::Packet packet;
    packet.set_allocated_file_header(header.release());

    Channel::sendMessage(packet);

    // the first chunk will get sent after the header reponse
    return true;
}

void FileChannel::acceptFile(tego_file_transfer_id_t id, const std::string& dest)
{
    auto it = incomingTransfers.find(id);
    TEGO_THROW_IF_FALSE(it != incomingTransfers.end());
    auto& itr = it->second;

    itr.beginTime = std::chrono::system_clock::now();
    itr.open_stream(dest);

    auto response = std::make_unique<Data::File::FileHeaderResponse>();
    response->set_response(tego_file_transfer_response_accept);
    response->set_file_id(id);

    Data::File::Packet packet;
    packet.set_allocated_file_header_response(response.release());
    Channel::sendMessage(packet);

    // emit starting transfer progress callback
    emit this->fileTransferProgress(id, tego_file_transfer_direction_receiving, 0, it->second.size);
}

void FileChannel::rejectFile(tego_file_transfer_id_t id)
{
    auto it = incomingTransfers.find(id);
    TEGO_THROW_IF_FALSE(it != incomingTransfers.end());

    // remove the incoming_transfer_record from our list on reject
    incomingTransfers.erase(it);

    auto response = std::make_unique<Data::File::FileHeaderResponse>();
    response->set_response(tego_file_transfer_response_reject);
    response->set_file_id(id);

    Data::File::Packet packet;
    packet.set_allocated_file_header_response(response.release());
    Channel::sendMessage(packet);

    // emit completion callback
    emit fileTransferFinished(id, tego_file_transfer_direction_receiving, tego_file_transfer_result_rejected);
}

bool FileChannel::cancelTransfer(tego_file_transfer_id_t id)
{
    // verify the transfer exists in our system
    switch(direction())
    {
    case Inbound:
        if (auto it = incomingTransfers.find(id); it != incomingTransfers.end())
        {
            incomingTransfers.erase(it);
        }
        else
        {
            return false;
        }
        break;
    case Outbound:
        if (auto it = outgoingTransfers.find(id); it != outgoingTransfers.end())
        {
            outgoingTransfers.erase(it);
        }
        else
        {
            return false;
        }
        break;
    default:
        return false;
    }

    // finally send cancel notification to remote user
    auto notification = std::make_unique<Data::File::FileTransferCompleteNotification>();
    notification->set_file_id(id);
    notification->set_result(Protocol::Data::File::Cancelled);

    Data::File::Packet packet;
    packet.set_allocated_file_transfer_complete_notification(notification.release());
    Channel::sendMessage(packet);

    emit fileTransferFinished(id, tego_file_transfer_direction_receiving, tego_file_transfer_result_cancelled);

    return true;
}

void FileChannel::sendNextChunk(tego_file_transfer_id_t id)
{
    Q_ASSERT(direction() == Outbound);

    if (auto it = outgoingTransfers.find(id); it != outgoingTransfers.end())
    {
        auto& otr = it->second;

        // make sure our offset and the stream offset agree
        Q_ASSERT(otr.finished() == false);
        Q_ASSERT(otr.offset == static_cast<tego_file_size_t>(otr.stream.tellg()));

        // read the next chunk to our buffer, and update our offset
        otr.stream.read(this->chunkBuffer, FileMaxChunkSize);
        const auto chunkSize = otr.stream.gcount();
        // ensure we read a valid value
        static_assert(FileMaxChunkSize != std::numeric_limits<std::streamsize>::max());
        if (chunkSize == std::numeric_limits<std::streamsize>::max())
        {
            // not quite a fatal error, but we need to cleanup this transfer
            emitNonFatalError("Problem reading the next chunk from disk", id, tego_file_transfer_result_filesystem_error);

            // send message to transfer partner to let them know we've given up
            auto notification = std::make_unique<Data::File::FileTransferCompleteNotification>();
            notification->set_file_id(id);
            notification->set_result(Protocol::Data::File::Cancelled);

            Data::File::Packet packet;
            packet.set_allocated_file_transfer_complete_notification(notification.release());
            Channel::sendMessage(packet);

            return;
        }
        Q_ASSERT(static_cast<tego_file_size_t>(chunkSize) <= FileMaxChunkSize);

        otr.offset += static_cast<unsigned long>(chunkSize);

        // build our chunk
        auto chunk = std::make_unique<Data::File::FileChunk>();
        chunk->set_file_id(id);
        chunk->set_chunk_data(std::begin(chunkBuffer), static_cast<size_t>(chunkSize));

        Data::File::Packet packet;
        packet.set_allocated_file_chunk(chunk.release());

        // send the chunk
        Channel::sendMessage(packet);
    }
}
