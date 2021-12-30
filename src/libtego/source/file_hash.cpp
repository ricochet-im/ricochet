#include "file_hash.hpp"
#include "error.hpp"

// implements deleter for openssl's EVP_MD_CTX
namespace std
{
    template<> class default_delete<::EVP_MD_CTX>
    {
    public:
        void operator()(EVP_MD_CTX* val)
        {
            ::EVP_MD_CTX_free(val);
        }
    };
}

tego_file_hash::tego_file_hash()
{
    TEGO_THROW_IF_FALSE(static_cast<size_t>(EVP_MD_size(EVP_sha3_512())) == data.size());
    data.fill(uint8_t(0x00));
}


tego_file_hash::tego_file_hash(uint8_t const* begin, uint8_t const* end)
: tego_file_hash()
{
    // init sha3 512 algo
    std::unique_ptr<::EVP_MD_CTX> ctx(EVP_MD_CTX_new());
    EVP_DigestInit_ex(ctx.get(), EVP_sha3_512(), nullptr);

    // calc hash
    EVP_DigestUpdate(ctx.get(), begin, static_cast<size_t>(end - begin));

    // copy hash to our local buffer
    uint32_t hashSize = 0;
    EVP_DigestFinal_ex(ctx.get(), data.begin(), &hashSize);
    TEGO_THROW_IF_FALSE(hashSize == this->DIGEST_SIZE);
}

tego_file_hash::tego_file_hash(std::istream& stream)
: tego_file_hash()
{
    // init sha3 512 algo
    std::unique_ptr<::EVP_MD_CTX> ctx(EVP_MD_CTX_new());
    EVP_DigestInit_ex(ctx.get(), EVP_sha3_512(), nullptr);

    // alloc a temp 64k buffer to read bytes into
    constexpr size_t BLOCK_SIZE = 65536;
    auto buffer = std::make_unique<char[]>(BLOCK_SIZE);

    // read and hash bytes
    while(stream.good())
    {
        // read bytes into buffer
        stream.read(buffer.get(), BLOCK_SIZE);
        const auto bytesRead = static_cast<size_t>(stream.gcount());
        TEGO_THROW_IF_FALSE_MSG(bytesRead <= BLOCK_SIZE, "Invalid amount of bytes read");

        // hash the block
        EVP_DigestUpdate(ctx.get(), buffer.get(), bytesRead);
    }

    // copy hash to our local buffer
    uint32_t hashSize = 0;
    EVP_DigestFinal_ex(ctx.get(), data.begin(), &hashSize);
    TEGO_THROW_IF_FALSE(hashSize == this->DIGEST_SIZE);
}

constexpr size_t tego_file_hash::string_size() const
{
    return STRING_SIZE;
}

const std::string& tego_file_hash::to_string() const
{
    if (hex.empty())
    {
        std::stringstream ss;
        for(auto byte : data)
        {
            fmt::print(ss, "{:02x}", byte);
        }
        hex = std::move(ss.str());
    }
    return hex;
}

extern "C"
{
    size_t tego_file_hash_string_size(
        tego_file_hash_t const* fileHash,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> size_t
        {
            TEGO_THROW_IF_NULL(fileHash);
            return fileHash->string_size();
        }, error, 0);
    }

     size_t tego_file_hash_to_string(
        tego_file_hash_t const* fileHash,
        char* out_hashString,
        size_t hashStringSize,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> size_t
        {
            TEGO_THROW_IF_NULL(fileHash);
            TEGO_THROW_IF_NULL(out_hashString);
            TEGO_THROW_IF_FALSE(hashStringSize >= fileHash->string_size());

            const auto& hashString = fileHash->to_string();
            std::copy(hashString.begin(), hashString.end(), out_hashString);
			// null terminator
            out_hashString[fileHash->string_size() - 1] = 0;

            return fileHash->string_size();
        }, error, 0);
    }
}
