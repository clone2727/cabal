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

#include <ApplicationServices/ApplicationServices.h>

#include "backends/fonts/ttf/ttf-provider.h" 
#include "backends/platform/darwin/cfref.h"
#include "common/debug.h"
#include "common/fs.h"
#include "common/stream.h"
#include "graphics/fonts/font-properties.h"

class CoreTextFontProvider : public TTFFontProvider {
protected:
	Common::SeekableReadStream *createReadStreamForFont(const Common::String &name, uint32 style);
};

namespace {

Common::String convertCFString(CFStringRef ref) {
	if (!ref)
		return "";

	// Lots of copying to ensure we have enough space...
	CFIndex length = CFStringGetLength(ref);
	CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
	char *buffer = new char[maxSize];
	CFStringGetCString(ref, buffer, maxSize, kCFStringEncodingUTF8);
	Common::String result = buffer;
	delete[] buffer;
	return result;
}

Common::String getPathFromCFURL(CFURLRef urlRef) {
	if (!urlRef)
		return "";

	char buf[PATH_MAX];
	if (!CFURLGetFileSystemRepresentation(urlRef, true, (UInt8 *)buf, sizeof(buf)))
		return "";

	return buf;
}

CFDictionaryRef createMatchDictionary(const Common::String &familyName, const Common::String &style) {
	// Create references for the two values
	ScopedCFRef<CFStringRef> familyNameRef(CFStringCreateWithCString(0, familyName.c_str(), kCFStringEncodingUTF8));
	if (!familyNameRef)
		return 0;

	ScopedCFRef<CFStringRef> styleNameRef(CFStringCreateWithCString(0, style.c_str(), kCFStringEncodingUTF8));
	if (!styleNameRef)
		return 0;

	// Create the arrays for the keys and values
	const void *keys[] = { kCTFontFamilyNameAttribute, kCTFontStyleNameAttribute };
	const void *values[] = { familyNameRef.get(), styleNameRef.get() };

	// Create the dictionary from the keys/values
	return CFDictionaryCreate(0, keys, values, ARRAYSIZE(keys), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
}

} // End of anonymous namespace

Common::SeekableReadStream *CoreTextFontProvider::createReadStreamForFont(const Common::String &name, uint32 style) {
	Common::String styleName = Graphics::getFontStyleString(style);

	// Create the dictionary defining the font we're looking for
	ScopedCFRef<CFDictionaryRef> dictRef(createMatchDictionary(name, styleName));
	if (!dictRef)
		return 0;

	// Create the font descriptor from that dictionary
	ScopedCFRef<CTFontDescriptorRef> fontDescriptor(CTFontDescriptorCreateWithAttributes(dictRef.get()));
	if (!fontDescriptor)
		return 0;

	// Turn that font descriptor into an array, which is what CTFontCollectionCreateWithFontDescriptors expects
	CTFontDescriptorRef tempFontDesc = fontDescriptor.get();
	ScopedCFRef<CFArrayRef> descriptors(CFArrayCreate(0, (CFTypeRef *)&tempFontDesc, 1, &kCFTypeArrayCallBacks));
	if (!descriptors)
		return 0;

	// Create a font collection from the array of descriptors
	ScopedCFRef<CTFontCollectionRef> collection(CTFontCollectionCreateWithFontDescriptors(descriptors.get(), 0));
	if (!collection)
		return 0;

	// Find the set of fonts that match the name/style we're looking for
	ScopedCFRef<CFArrayRef> matchingDescriptors(CTFontCollectionCreateMatchingFontDescriptors(collection.get()));
	if (!matchingDescriptors)
		return 0;

	// Get the count of fonts
	CFIndex count = CFArrayGetCount(matchingDescriptors.get());
	for (CFIndex i = 0; i < count; i++) {
		CTFontDescriptorRef descriptor = (CTFontDescriptorRef)CFArrayGetValueAtIndex(matchingDescriptors.get(), i);

		// Get the family name for the font. We don't want a broken fallback.
		ScopedCFRef<CFStringRef> nameRef((CFStringRef)CTFontDescriptorCopyAttribute(descriptor, kCTFontFamilyNameAttribute));
		if (!nameRef || !name.equalsIgnoreCase(convertCFString(nameRef.get())))
			continue;

		// Get the URL for the font
		ScopedCFRef<CFURLRef> urlRef((CFURLRef)CTFontDescriptorCopyAttribute(descriptor, kCTFontURLAttribute));
		if (!urlRef)
			continue;

		// Convert the URL into a string
		Common::String fileName = getPathFromCFURL(urlRef.get());
		if (fileName.empty())
			continue;

		// Check that the file exists
		Common::FSNode node(fileName);
		if (!node.exists() || node.isDirectory())
			continue;

		// Create a read stream for the font
		// Streams with size 0 are suitcase fonts which aren't currently supported
		Common::ScopedPtr<Common::SeekableReadStream> stream(node.createReadStream());
		if (stream && stream->size() > 0)
			return stream.release();
	}

	// Failed to find a font
	return 0;
}

SystemFontProvider *createCoreTextFontProvider() {
	return new CoreTextFontProvider();
}
