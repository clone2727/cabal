/* Cabal - Legacy Game Implementations
 *
 * Cabal is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// Based on the ScummVM (GPLv2+) file of the same name

/**
 * @file
 * Macintosh resource fork manager used in engines:
 * - groovie
 * - mohawk
 * - pegasus
 * - sci
 * - scumm
 */

#ifndef COMMON_MACRESMAN_H
#define COMMON_MACRESMAN_H

#include "common/array.h"
#include "common/hashmap.h"
#include "common/str.h"

namespace Common {

class FSNode;
class SeekableReadStream;

typedef Array<uint16> MacResIDArray;
typedef Array<uint32> MacResTagArray;

/**
 * Class for handling Mac data and resource forks.
 * It can read from raw, MacBinary, and AppleDouble formats.
 */
class MacResManager {

public:
	MacResManager();
	~MacResManager();

	/**
	 * Open a Mac data/resource fork pair.
	 *
	 * This uses SearchMan to find the data/resource forks. This should only be used
	 * from inside an engine.
	 *
	 * @param fileName The base file name of the file
	 * @note This will check for the raw resource fork, MacBinary, and AppleDouble formats.
	 * @return True on success
	 */
	bool open(const String &fileName);

	/**
	 * Open a Mac data/resource fork pair.
	 *
	 * @param path The path that holds the forks
	 * @param fileName The base file name of the file
	 * @note This will check for the raw resource fork, MacBinary, and AppleDouble formats.
	 * @return True on success
	 */
	bool open(const FSNode &path, const String &fileName);

	/**
	 * See if a Mac data/resource fork pair exists.
	 * @param fileName The base file name of the file
	 * @return True if either a data fork or resource fork with this name exists
	 */
	static bool exists(const String &fileName);

	/**
	 * Close the Mac data/resource fork pair.
	 */
	void close();

	/**
	 * Query whether or not we have a data fork present.
	 * @return True if the data fork is present
	 */
	bool hasDataFork() const;

	/**
	 * Query whether or not we have a data fork present.
	 * @return True if the resource fork is present
	 */
	bool hasResFork() const;

	/**
	 * Read resource from the MacBinary file
	 * @param typeID FourCC of the type
	 * @param resID Resource ID to fetch
	 * @return Pointer to a SeekableReadStream with loaded resource
	 */
	SeekableReadStream *getResource(uint32 typeID, uint16 resID);

	/**
	 * Read resource from the MacBinary file
	 * @note This will take the first resource that matches this name, regardless of type
	 * @param filename file name of the resource
	 * @return Pointer to a SeekableReadStream with loaded resource
	 */
	SeekableReadStream *getResource(const String &filename);

	/**
	 * Read resource from the MacBinary file
	 * @param typeID FourCC of the type
	 * @param filename file name of the resource
	 * @return Pointer to a SeekableReadStream with loaded resource
	 */
	SeekableReadStream *getResource(uint32 typeID, const String &filename);

	/**
	 * Retrieve the data fork
	 * @return The stream if present, 0 otherwise
	 */
	SeekableReadStream *getDataFork();

	/**
	 * Get the name of a given resource
	 * @param typeID FourCC of the type
	 * @param resID Resource ID to fetch
	 * @return The name of a given resource and an empty string if not present
	 */
	String getResName(uint32 typeID, uint16 resID) const;

	/**
	 * Get the size of the data portion of the resource fork
	 * @return The size of the data portion of the resource fork
	 */
	uint32 getResForkDataSize() const;

	/**
	 * Calculate the MD5 checksum of the resource fork
	 * @param length The maximum length to compute for
	 * @return The MD5 checksum of the resource fork
	 */
	String computeResForkMD5AsString(uint32 length = 0) const;

	/**
	 * Get the base file name of the data/resource fork pair
	 * @return The base file name of the data/resource fork pair
	 */
	String getBaseFileName() const { return _baseFileName; }

	/**
	 * Return list of resource IDs with specified type ID
	 */
	MacResIDArray getResIDArray(uint32 typeID);

	/**
	 * Return list of resource tags
	 */
	MacResTagArray getResTagArray();

private:
	SeekableReadStream *_stream;
	String _baseFileName;

	bool load(SeekableReadStream &stream);

	bool loadFromRawFork(SeekableReadStream &stream);
	bool loadFromMacBinary(SeekableReadStream &stream);
	bool loadFromAppleDouble(SeekableReadStream &stream);

	/**
	 * Given a path, create the path where Mac OS X would place the AppleDouble file
	 * containing the resource fork (regardless whether or not it exists)
	 */
	static String constructAppleDoubleName(const String &path);

	/**
	 * Check if the given stream is in the MacBinary format.
	 * @param stream The stream we're checking
	 */
	static bool isMacBinary(SeekableReadStream &stream);

	enum {
		kResForkNone = 0,
		kResForkRaw,
		kResForkMacBinary,
		kResForkAppleDouble
	} _mode;

	void readMap();

	struct Resource {
		byte attr;
		uint32 dataOffset;
		String name;
	};

	typedef Common::HashMap<uint16, Resource> ResourceMap;
	typedef Common::HashMap<uint32, ResourceMap> ResourceTypeMap;

	ResourceTypeMap _resTypeMap;

	int32 _resForkOffset;
	uint32 _resForkSize;

	uint32 _dataOffset;
	uint32 _dataLength;
	uint32 _mapOffset;
	uint32 _mapLength;
};

} // End of namespace Common

#endif
