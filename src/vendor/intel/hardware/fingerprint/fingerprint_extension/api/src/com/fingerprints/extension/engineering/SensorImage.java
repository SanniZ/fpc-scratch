/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.engineering;

/**
 * Image captured from a fingerprint sensor. The image is represented as an uncompressed bitmap.
 */
public class SensorImage {

    /**
     * List of supported pixel depths
     */
    public enum BitsPerPixel {
        /**
         * 8 bits per pixel
         */
        BPP_8
    }

    private BitsPerPixel mBitsPerPixel;
    private int mWidth;
    private int mHeight;
    private byte[] mPixels;

    /**
     * Creates an image.
     *
     * @param bitsPerPixel Pixel depth
     * @param width        Image width (in pixels)
     * @param height       Image height (in pixels)
     * @param pixels       Byte array of pixels.
     */
    public SensorImage(BitsPerPixel bitsPerPixel, int width, int height, byte[] pixels) {
        mBitsPerPixel = bitsPerPixel;
        mWidth = width;
        mHeight = height;
        mPixels = pixels;
    }

    /**
     * Returns pixel depth (bits per pixel)
     */
    public BitsPerPixel getBitsPerPixel() {
        return mBitsPerPixel;
    }

    /**
     * Returns the width (in pixels).
     */
    public int getWidth() {
        return mWidth;
    }

    /**
     * Returns the height (in pixels).
     */
    public int getHeight() {
        return mHeight;
    }

    /**
     * Returns a byte array with the pixels.
     */
    public byte[] getPixels() {
        return mPixels;
    }
}
