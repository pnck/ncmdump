#include "ncmcrypt.h"
#include "aes.h"
#include "base64.h"
#include "cJSON.h"

#include <taglib/mpegfile.h>
#include <taglib/flacfile.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/id3v2tag.h>
#include <taglib/tag.h>

#include <string>
#include <cstring>

using namespace ncmdump;

const uint8_t NeteaseCrypt::sCoreKey[] =
    {0x68, 0x7A, 0x48, 0x52, 0x41, 0x6D, 0x73, 0x6F, 0x35, 0x6B, 0x49, 0x6E, 0x62, 0x61, 0x78, 0x57};
const uint8_t NeteaseCrypt::sModifyKey[] =
    {0x23, 0x31, 0x34, 0x6C, 0x6A, 0x6B, 0x5F, 0x21, 0x5C, 0x5D, 0x26, 0x30, 0x55, 0x3C, 0x27, 0x28};

const uint8_t NeteaseCrypt::mPng[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

static void replace(std::string &str, const std::string &from, const std::string &to) {
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

static std::string fileNameWithoutExt(const std::string &str) {
    size_t lastPath = str.find_last_of("/\\");
    std::string path = str.substr(lastPath + 1);
    size_t lastExt = path.find_last_of(".");
    return path.substr(0, lastExt);
}

NeteaseMusicMetadata::~NeteaseMusicMetadata() {
    cJSON_Delete(mRaw);
}

NeteaseMusicMetadata::NeteaseMusicMetadata(cJSON *raw) {
    if (!raw) {
        return;
    }
    mRaw = raw;
/*
    char *_meta = cJSON_Print(raw);
    std::cout << "METADATA>>\n" << _meta << std::endl;
    free(_meta);
*/
    cJSON *member = nullptr;
    cJSON_ArrayForEach(member, raw) {
        char *field = member->string;
        if (!strcasecmp(field, "musicName")) {
            mName = cJSON_GetStringValue(member);
            continue;
        } else if (!strcasecmp(field, "album")) {
            mAlbum = cJSON_GetStringValue(member);
            continue;
        } else if (!strcasecmp(field, "artist")) {
            cJSON *artist = nullptr;
            cJSON_ArrayForEach(artist, member) {
                if (mArtist.length()) {
                    mArtist += "/";
                }
                mArtist += cJSON_GetStringValue(cJSON_GetArrayItem(artist, 0));
            }
            continue;
        } else if (!strcasecmp(field, "bitrate")) {
            mBitrate = member->valueint;
            continue;
        } else if (!strcasecmp(field, "duration")) {
            mDuration = member->valueint;
            continue;
        } else if (!strcasecmp(field, "format")) {
            mFormat = cJSON_GetStringValue(member);
            continue;
        }
    }
}

bool NeteaseCrypt::openFile(std::string const &path) {
    try {
        mFile.open(path, std::ios::in | std::ios::binary);
    } catch (...) {
        return false;
    }
    return bool(mFile);
}

bool NeteaseCrypt::isNcmFile() {
    const uint64_t *_hd = reinterpret_cast<const uint64_t *>("CTENFDAM");
    char header[8];
    mFile.read(header, sizeof(header));
    return *_hd == *reinterpret_cast<uint64_t *>(header);
}

std::streamsize NeteaseCrypt::read(void *p, std::streamsize n) {
    mFile.read(reinterpret_cast<char *>(p), n);
    std::streamsize gcount = mFile.gcount();
    if (gcount <= 0) {
        throw std::runtime_error("can't read file");
    }
    return gcount;
}

void NeteaseCrypt::buildRC4KeyBox(uint8_t *key, size_t keyLen) {
    int i;
    for (i = 0; i < 256; ++i) {
        mRC4KeyBox[i] = (uint8_t) i;
    }

    uint8_t last = 0;
    uint8_t key_index = 0;

    for (i = 0; i < 256; ++i) {
        uint8_t tmp = mRC4KeyBox[i];
        last = static_cast<uint8_t>((tmp + last + key[key_index++ % keyLen]) & 0xff);
        mRC4KeyBox[i] = mRC4KeyBox[last];
        mRC4KeyBox[last] = tmp;
    }
}

std::string NeteaseCrypt::mimeType(std::string &data) {
    if (memcmp(data.c_str(), mPng, 8) == 0) {
        return "image/png";
    }

    return "image/jpeg";
}

void NeteaseCrypt::FixMetadata() {
    if (mDumpFilepath.length() <= 0) {
        throw std::invalid_argument("must dump before");
    }

    TagLib::File *audioFile;
    TagLib::Tag *tag;
    TagLib::ByteVector vector(mImageData.c_str(), mImageData.length());

    if (mFormat == NeteaseCrypt::MP3) {
        audioFile = new TagLib::MPEG::File(mDumpFilepath.c_str());
        tag = dynamic_cast<TagLib::MPEG::File *>(audioFile)->ID3v2Tag(true);

        if (mImageData.length() > 0) {
            TagLib::ID3v2::AttachedPictureFrame *frame = new TagLib::ID3v2::AttachedPictureFrame;

            frame->setMimeType(mimeType(mImageData));
            frame->setPicture(vector);

            dynamic_cast<TagLib::ID3v2::Tag *>(tag)->addFrame(frame);
        }
    } else if (mFormat == NeteaseCrypt::FLAC) {
        audioFile = new TagLib::FLAC::File(mDumpFilepath.c_str());
        tag = audioFile->tag();

        if (mImageData.length() > 0) {
            TagLib::FLAC::Picture *cover = new TagLib::FLAC::Picture;
            cover->setMimeType(mimeType(mImageData));
            cover->setType(TagLib::FLAC::Picture::FrontCover);
            cover->setData(vector);

            dynamic_cast<TagLib::FLAC::File *>(audioFile)->addPicture(cover);
        }
    }

    if (mMetaData != NULL) {
        tag->setTitle(TagLib::String(mMetaData->name(), TagLib::String::UTF8));
        tag->setArtist(TagLib::String(mMetaData->artist(), TagLib::String::UTF8));
        tag->setAlbum(TagLib::String(mMetaData->album(), TagLib::String::UTF8));
    }

    tag->setComment(TagLib::String("Create by netease copyright protected dump tool. author 5L", TagLib::String::UTF8));

    audioFile->save();
}

void NeteaseCrypt::Dump() {
    int n, i;

    // mDumpFilepath.clear();
    // mDumpFilepath.resize(1024);

    // if (mMetaData) {
    // 	mDumpFilepath = mMetaData->name();

    // 	replace(mDumpFilepath, "\\", "＼");
    // 	replace(mDumpFilepath, "/", "／");
    // 	replace(mDumpFilepath, "?", "？");
    // 	replace(mDumpFilepath, ":", "：");
    // 	replace(mDumpFilepath, "*", "＊");
    // 	replace(mDumpFilepath, "\"", "＂");
    // 	replace(mDumpFilepath, "<", "＜");
    // 	replace(mDumpFilepath, ">", "＞");
    // 	replace(mDumpFilepath, "|", "｜");
    // } else {
    mDumpFilepath = fileNameWithoutExt(mFilepath);
    // }

    n = 0x8000;
    i = 0;

    uint8_t buffer[n];

    std::ofstream output;

    while (!mFile.eof()) {
        n = read((char *) buffer, n);

        for (i = 0; i < n; i++) {
            int j = (i + 1) & 0xff;
            buffer[i] ^= mRC4KeyBox[(mRC4KeyBox[j] + mRC4KeyBox[(mRC4KeyBox[j] + j) & 0xff]) & 0xff];
        }

        if (!output.is_open()) {
            // identify format
            // ID3 format mp3
            if (buffer[0] == 0x49 && buffer[1] == 0x44 && buffer[2] == 0x33) {
                mDumpFilepath += ".mp3";
                mFormat = NeteaseCrypt::MP3;
            } else {
                mDumpFilepath += ".flac";
                mFormat = NeteaseCrypt::FLAC;
            }

            output.open(mDumpFilepath, output.out | output.binary);
        }

        output.write((char *) buffer, n);
    }

    output.flush();
    output.close();
}

NeteaseCrypt::~NeteaseCrypt() {
    if (mMetaData != NULL) {
        delete mMetaData;
    }

    mFile.close();
}

NeteaseCrypt::NeteaseCrypt(const std::string &path) {
    if (!openFile(path)) {
        throw std::invalid_argument("can't open file");
    }

    if (!isNcmFile()) {
        throw std::invalid_argument("not netease protected file");
    }

    if (!mFile.seekg(2, std::ifstream::cur)) {
        throw std::invalid_argument("can't seek file");
    }

    mFilepath = path;
    uint32_t read_len = 0;
    read(&read_len, sizeof(read_len));

    if (read_len <= 0) {
        throw std::invalid_argument("broken ncm file");
    }
    std::vector<uint8_t> key_data(read_len);
    read(key_data.data(), read_len);
    for (auto &c:key_data) {
        c ^= 0x64;
    }

    key_data = aes::ecb_decrypt(sCoreKey, key_data.data(), read_len);
    // looks like `neteasecloudmusic1226132383402E7fT49x7dof9OKCgg9cdvhEuezy3iZCL1nFvBFd1T4uSktAJKmwZXsijPbijliionVUXXg9plTbXEclAE9Lb`
    // skip `neteasecloudmusic` header
    buildRC4KeyBox(key_data.data() + 17, key_data.size() - 17);

    uint32_t meta_len = 0;
    read(&meta_len, sizeof(meta_len));

    if (meta_len <= 0) {
        fprintf(stderr, "[Warn] missing metadata of `%s` !\n", path.c_str());
    } else {
        std::vector<uint8_t> meta_content(meta_len);
        read(meta_content.data(), meta_len);
        for (auto &c:meta_content) {
            c ^= 0x63;
        }

        // looks like `163 key(Don't modify):L64FU3W4YxX3ZFTmbZ+8/ePJkc0Fh/R6...`
        // skip `163 key(Don't modify):`
        meta_content = base64::decode(reinterpret_cast<char *>(meta_content.data() + 22), meta_len - 22);
        meta_content = aes::ecb_decrypt(sModifyKey, meta_content.data(), meta_content.size());

        // skip `music:`
        auto meta_info = std::string(meta_content.begin() + 6, meta_content.end());

        // std::cout << modifyDecryptData << std::endl;

        mMetaData = new NeteaseMusicMetadata(cJSON_Parse(meta_info.c_str()));
    }

    // skip crc32 & unuse charset
    if (!mFile.seekg(9, std::ifstream::cur)) {
        throw std::invalid_argument("can't seek file");
    }

    read(&read_len, sizeof(read_len));

    if (read_len > 0) {
        char *imageData = (char *) malloc(read_len);
        read(imageData, read_len);

        mImageData = std::string(imageData, read_len);
    } else {
        printf("[Warn] `%s` missing album can't fix album image!\n", path.c_str());
    }
}
