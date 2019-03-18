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

import java.util.ArrayList;

public class FMI {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private FMIFile mFMIFile;

    public void FMI() {
    }

    public FMIStatus openFile(String path, FMIOpenMode mode) {
        mLogger.enter("openFile");
        FMIStatus status = FMIStatus.OK;

        if (path != null && mode != null) {
            mFMIFile = new FMIFile();
            status = mFMIFile.openFile(path, mode);
            if (status != FMIStatus.OK) {
                mFMIFile = null;
            }
        } else {
            mLogger.e("Invalid argument");
            status = FMIStatus.INVALID_ARGUMENT;
        }

        mLogger.exit("openFile");
        return status;
    }

    public FMIStatus addBlock(FMIBlockType type, byte[] data) {
        mLogger.enter("addBlock");
        FMIStatus status = FMIStatus.OK;

        if (type != null && data != null && mFMIFile != null) {
            status = mFMIFile.addBlock(type, data);
        } else {
            mLogger.e("Invalid argument");
            status = FMIStatus.INVALID_ARGUMENT;
        }

        mLogger.exit("addBlock");
        return status;
    }

    public FMIStatus getBlockData(FMIBlock block, byte[] data) {
        mLogger.enter("getBlockData");
        FMIStatus status = FMIStatus.OK;

        if (block != null && data != null && mFMIFile != null) {
            status = mFMIFile.getBlockData(block, data);
        } else {
            mLogger.e("Invalid argument");
            status = FMIStatus.INVALID_ARGUMENT;
        }

        mLogger.exit("getBlockData");
        return status;
    }

    public ArrayList<FMIBlock> getBlocks() {
        if (mFMIFile != null) {
            return mFMIFile.getBlocks();
        }
        return null;
    }

    public long getSize() {
        long size = 0;
        if (mFMIFile != null) {
            size = mFMIFile.getSize();
        }
        return size;
    }

    public FMIStatus close() {
        mLogger.enter("close");
        FMIStatus status = FMIStatus.OK;
        if (mFMIFile != null) {
            status = mFMIFile.close();
        }
        mLogger.exit("close");
        return status;
    }
}

