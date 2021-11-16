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

#ifndef PROTOCOL_FILECHANNEL_H
#define PROTOCOL_FILECHANNEL_H

#include "protocol/Channel.h"
#include "FileChannel.pb.h"
#include "tego/tego.h"
#include "file_hash.hpp"

namespace Protocol
{

class FileChannel : public Channel
{
    Q_OBJECT;
    Q_DISABLE_COPY(FileChannel);

public:
    explicit FileChannel(Direction direction, Connection *connection);

    bool sendFileWithId(QString file_url, const tego_file_hash_t& fileHash, QDateTime time, tego_file_transfer_id_t id);
    void acceptFile(tego_file_transfer_id_t id, const std::string& dest);
    void rejectFile(tego_file_transfer_id_t id);
    bool cancelTransfer(tego_file_transfer_id_t id);
    // signals bubble up to the ConversationModel object that owns this FileChannel
signals:
    void fileTransferRequestReceived(tego_file_transfer_id_t id, QString fileName, tego_file_size_t fileSize, tego_file_hash_t);
    void fileTransferAcknowledged(tego_file_transfer_id_t id, bool ack);
    void fileTransferRequestResponded(tego_file_transfer_id_t id, tego_file_transfer_response_t response);
    void fileTransferProgress(tego_file_transfer_id_t id, tego_file_transfer_direction_t direction, tego_file_size_t bytesTransmitted, tego_file_size_t bytesTotal);
    void fileTransferFinished(tego_file_transfer_id_t id, tego_file_transfer_direction_t direction, tego_file_transfer_result_t);

protected:
    virtual bool allowInboundChannelRequest(const Data::Control::OpenChannel *request, Data::Control::ChannelResult *result);
    virtual bool allowOutboundChannelRequest(Data::Control::OpenChannel *request);
    virtual void receivePacket(const QByteArray &packet);
private:
    // when our socket goes away
    void onConnectionClosed();

    // we need runtime checks to ensure that sizes stored as tego_file_size_t are representable as
    // std::streamoff too where appropriate
    // verify that std::streamoff is representable as a tego_file_size_t
    static_assert(std::numeric_limits<std::streamoff>::max() <= std::numeric_limits<tego_file_size_t>::max());
    // verify the QFileInfo::size() method returns a qint64
    static_assert(std::is_same_v<decltype(QFileInfo().size()), qint64>);
    // verify that std::streamoff is representable as qint64 (type used by Qt File APIs for sizes)
    static_assert(std::numeric_limits<std::streamoff>::max() <= std::numeric_limits<qint64>::max());

    struct outgoing_transfer_record
    {
        outgoing_transfer_record(
            tego_file_transfer_id_t id,
            const std::string& filePath,
            tego_file_size_t fileSize);

        std::chrono::time_point<std::chrono::system_clock> beginTime;

        const tego_file_transfer_id_t id;
        const tego_file_size_t size;
        tego_file_size_t offset;
        std::ifstream stream;

        inline bool finished() const { return offset == size; }
    };

    struct incoming_transfer_record
	{
        incoming_transfer_record(
            tego_file_transfer_id_t id,
            tego_file_size_t fileSize,
            const std::string& fileHash);
        // explicit destructor defined, so we need to explicitly define a move constructor
		// for usage with std::map
        incoming_transfer_record(incoming_transfer_record&&) = default;

        ~incoming_transfer_record();

        std::chrono::time_point<std::chrono::system_clock> beginTime;

        const tego_file_transfer_id_t id;
        const tego_file_size_t size;
        std::string dest; // destination to save to
        const std::string hash;

        // need to write and read
        std::fstream stream;

        std::string partial_dest() const;
        void open_stream(const std::string& dest);
    };
    // 63 kb, max packet size is UINT16_MAX (ak 65535, 64k - 1) so leave space for other data
    constexpr static tego_file_size_t FileMaxChunkSize = 63*1024; // bytes
    // intermediate buffer we load chunks from disk into
    // each access to this buffer happens on the same thread, and only within the scope of a function
    // so no need to worry about synchronization or sharing between file transfers
    char chunkBuffer[FileMaxChunkSize];

    // file transfers we are sending
    std::map<tego_file_transfer_id_t, outgoing_transfer_record> outgoingTransfers;
    // file transfers we are receiving
    std::map<tego_file_transfer_id_t, incoming_transfer_record> incomingTransfers;

    // called when something unrecoverable occurs, or contact is sending us bad packets, or we get in
    // some other allegedly impossible state; kills all our transfers and disconnect the channel
    void emitFatalError(std::string&& msg, tego_file_transfer_result_t error, bool shouldCloseChannel);
    // called when some error occurs that does not affect other transfers
    void emitNonFatalError(std::string&& msg, tego_file_transfer_id_t id, tego_file_transfer_result_t error);

    bool verifyPacket(Data::File::Packet const& message);
    bool verifyFileHeader(Data::File::FileHeader const& message);
    bool verifyFileHeaderAck(Data::File::FileHeaderAck const& message);
    bool verifyFileHeaderResponse(Data::File::FileHeaderResponse const& message);
    bool verifyFileChunk(Data::File::FileChunk const& message);
    bool verifyFileChunkAck(Data::File::FileChunkAck const& message);
    bool verifyFileTransferCompleteNotification(Data::File::FileTransferCompleteNotification const& message);

    void handleFileHeader(const Data::File::FileHeader &message);
    void handleFileHeaderAck(const Data::File::FileHeaderAck &message);
    void handleFileHeaderResponse(const Data::File::FileHeaderResponse &message);
    void handleFileChunk(const Data::File::FileChunk &message);
    void handleFileChunkAck(const Data::File::FileChunkAck &message);
    void handleFileTransferCompleteNotification(const Data::File::FileTransferCompleteNotification &message);

    void sendNextChunk(tego_file_transfer_id_t id);
};

}
#endif
