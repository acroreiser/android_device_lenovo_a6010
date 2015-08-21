/*
 * Copyright (C) 2015 The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.cyanogenmod.hardware;

import android.util.Log;

import cyanogenmod.hardware.DisplayMode;

/*
 * Display Modes API
 *
 * A device may implement a list of preset display modes for different
 * viewing intents, such as movies, photos, or extra vibrance. These
 * modes may have multiple components such as gamma correction, white
 * point adjustment, etc, but are activated by a single control point.
 *
 * This API provides support for enumerating and selecting the
 * modes supported by the hardware.
 */

public class DisplayModeControl {

    private static boolean sNativeLibraryLoaded;

    static {
        try {
            System.loadLibrary("jni_livedisplay_qdcm");
            sNativeLibraryLoaded = true;
        } catch (Throwable t) {
            sNativeLibraryLoaded = false;
            Log.w("CMHW", "Display mode support unavailable.");
        }
    }

    private static native boolean native_isSupported();
    private static native DisplayMode[] native_getDisplayModes();
    private static native DisplayMode native_getCurrentDisplayMode();
    private static native DisplayMode native_getDefaultDisplayMode();
    private static native boolean native_setDisplayMode(DisplayMode mode, boolean makeDefault);

    /*
     * All HAF classes should export this boolean.
     * Real implementations must, of course, return true
     */
    public static boolean isSupported() {
        return sNativeLibraryLoaded && native_isSupported();
    }

    /*
     * Get the list of available modes. A mode has an integer
     * identifier and a string name.
     *
     * It is the responsibility of the upper layers to
     * map the name to a human-readable format or perform translation.
     */
    public static DisplayMode[] getAvailableModes() {
        if (!sNativeLibraryLoaded) {
            return new DisplayMode[0];
        }
        return native_getDisplayModes();
    }

    /*
     * Get the name of the currently selected mode. This can return
     * null if no mode is selected.
     */
    public static DisplayMode getCurrentMode() {
        if (!sNativeLibraryLoaded) {
            return null;
        }
        return native_getCurrentDisplayMode();
    }

    /*
     * Selects a mode from the list of available modes by it's
     * string identifier. Returns true on success, false for
     * failure. It is up to the implementation to determine
     * if this mode is valid.
     */
    public static boolean setMode(DisplayMode mode, boolean makeDefault) {
        if (!sNativeLibraryLoaded) {
            return false;
        }
        return native_setDisplayMode(mode, makeDefault);
    }

    /*
     * Gets the preferred default mode for this device by it's
     * string identifier. Can return null if there is no default.
     */
    public static DisplayMode getDefaultMode() {
        if (!sNativeLibraryLoaded) {
            return null;
        }
        return native_getDefaultDisplayMode();
    }
}
