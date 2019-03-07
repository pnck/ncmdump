#pragma once

#include "aes.h"
#include "cJSON.h"

#include <iostream>
#include <fstream>

namespace ncmdump {
class NeteaseMusicMetadata;
class NeteaseCrypt;
}

class ncmdump::NeteaseMusicMetadata {

private:
	std::string mAlbum;
	std::string mArtist;
	std::string mFormat;
	std::string mName;
	int mDuration;
	int mBitrate;

private:
	cJSON* mRaw;

public:
	explicit NeteaseMusicMetadata(cJSON*);
	~NeteaseMusicMetadata();
    const std::string& name() const { return mName; }
    const std::string& album() const { return mAlbum; }
    const std::string& artist() const { return mArtist; }
    const std::string& format() const { return mFormat; }
    const int duration() const { return mDuration; }
    const int bitrate() const { return mBitrate; }

};

class ncmdump::NeteaseCrypt {

private:
	static const uint8_t sCoreKey[17];
	static const uint8_t sModifyKey[17];
	static const uint8_t mPng[8];
	enum NcmFormat { MP3, FLAC };

private:
	std::string mFilepath;
	std::string mDumpFilepath;
	NcmFormat mFormat;
	std::string mImageData;
	std::ifstream mFile;
	uint8_t mKeyBox[256];
	NeteaseMusicMetadata* mMetaData;

private:
	bool isNcmFile();
	bool openFile(std::string const&);
	int read(char *s, std::streamsize n);
	void buildKeyBox(uint8_t *key, int keyLen);
	std::string mimeType(std::string& data);

public:
	const std::string& filepath() const { return mFilepath; }
	const std::string& dumpFilepath() const { return mDumpFilepath; }

public:
	explicit NeteaseCrypt(std::string const&);
	~NeteaseCrypt();

public:
	void Dump();
	void FixMetadata();
};