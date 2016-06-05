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

#ifndef BACKENDS_PLATFORM_DARWIN_CFREF_H
#define BACKENDS_PLATFORM_DARWIN_CFREF_H

#include "common/scummsys.h"
#include <CoreFoundation/CoreFoundation.h>

/**
 * Class to handle CoreFoundation references in an RAII-compatible way.
 * Intended to be used similarly to Common::ScopedPtr.
 */
template<typename T>
class ScopedCFRef {
public:
	typedef T ReferenceType;

	explicit ScopedCFRef(T ref = 0) : _ref(ref) {}

	~ScopedCFRef() {
		if (_ref)
			CFRelease(_ref);
	}

	/**
	 * Helper bool conversion operator
	 */
	CABAL_EXPLICIT_OPERATOR_CONV operator bool() const {
		return _ref != 0;
	}

	/**
	 * Get the reference this object manages
	 *
	 * @return the object the ScopedCFRef manage
	 */
	T get() const {
		return _ref;
	}

	/**
	 * Returns the plain pointer value and releases ScopedCFRef.
	 * After release() call you need to call CFRelease on the object yourself.
	 *
	 * @return the object the ScopedCFRef managed
	 */
	T release() {
		T tmp = _ref;
		_ref = 0;
		return tmp;
	}

	/**
	 * Reset the reference with another. The previous reference has CFRelease()
	 * called on it if it was non-NULL.
	 */
	void reset(T ref = 0) {
		if (_ref)
			CFRelease(_ref);

		_ref = ref;
	}

private:
	/**
	 * The captured reference
	 */
	T _ref;
};

#endif
