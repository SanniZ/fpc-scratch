/*
*
* Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
*
* All rights are reserved.
* Proprietary and confidential.
* Unauthorized copying of this file, via any medium is strictly prohibited.
* Any use is subject to an appropriate license granted by Fingerprint Cards AB.
*
*/

package com.fingerprints.fmi;

public class Constants {
    public static final byte[] SIGNATURE = new byte[]{0x2E, 0x46, 0x4D, 0x49};
    public static final int VERSION_MAJOR = 1;
    public static final int VERSION_MINOR = 0;
    public static final int HEADER_SIGNATURE_SIZE = 4;
    public static final int HEADER_SIZE = 4;
    public static final int HEADER_VERSION_SIZE = 4;
    public static final int TOTAL_HEADER_SIZE = HEADER_SIGNATURE_SIZE + HEADER_SIZE + HEADER_VERSION_SIZE;
    public static final int BLOCK_TYPE_SIZE = 4;
    public static final int BLOCK_PAYLOAD_SIZE = 4;
    public static final int TOTAL_BLOCK_HEADER_SIZE = BLOCK_TYPE_SIZE + BLOCK_PAYLOAD_SIZE;
}
