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

import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.ArrayList;

public class FMIFile {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private RandomAccessFile mFile;
    private FMIOpenMode mMode;
    private FMIHeader mHeader = new FMIHeader();
    private ArrayList<FMIBlock> mBlocks = new ArrayList<FMIBlock>();

    public void FMIFile() {
    }

    public FMIStatus openFile(String path, FMIOpenMode mode) {
        mLogger.enter("openFile");
        FMIStatus status = FMIStatus.OK;

        mMode = mode;
        if (mMode == FMIOpenMode.READ) {
            try {
                mFile = new RandomAccessFile(path, "r");
                status = readHeader();
                if (status == FMIStatus.OK) {
                    status = readBlocks();
                }
            } catch (IOException e) {
                status = FMIStatus.FAILED_IO;
                mLogger.e("IOException: " + e);
            }
        } else if (mMode == FMIOpenMode.WRITE) {
            try {
                mFile = new RandomAccessFile(path, "rw");
                writeHeader();
            } catch (IOException e) {
                status = FMIStatus.FAILED_IO;
                mLogger.e("IOException: " + e);
            }
        } else {
            status = FMIStatus.INVALID_ARGUMENT;
        }

        mLogger.exit("openFile");
        return status;
    }

    public FMIStatus addBlock(FMIBlockType type, byte[] data) {
        mLogger.enter("addBlock");
        FMIStatus status = FMIStatus.OK;
        if (mMode == FMIOpenMode.WRITE) {
            try {
                mFile.writeShort(LE((short) type.format.getValue()));
                mFile.writeShort(LE((short) type.id.getValue()));
                mFile.writeInt(LE(data.length));
                mFile.write(data);
            } catch (IOException e) {
                status = FMIStatus.FAILED_IO;
                mLogger.e("IOException: " + e);
            }
        } else {
            status = FMIStatus.UNSUPPORTED;
        }

        mLogger.exit("addBlock");
        return status;
    }

    public FMIStatus getBlockData(FMIBlock block, byte[] data) {
        mLogger.enter("getBlockData");
        FMIStatus status = FMIStatus.OK;
        if (mMode == FMIOpenMode.READ) {
            try {
                mFile.seek(block.offset);
                int length = mFile.read(data);
                mLogger.i("length: " + length);
            } catch (IOException e) {
                status = FMIStatus.FAILED_IO;
                mLogger.e("IOException: " + e);
            }
        } else {
            status = FMIStatus.UNSUPPORTED;
        }

        mLogger.exit("getBlockData");
        return status;
    }

    public long getSize() {
        mLogger.enter("getSize");
        long size = 0;
        try {
            size = mFile.length();
        } catch (IOException e) {
            mLogger.e("IOException: " + e);
        }
        mLogger.exit("getSize");
        return size;
    }

    public FMIStatus close() {
        mLogger.enter("close");
        FMIStatus status = FMIStatus.OK;
        try {
            mFile.close();
        } catch (IOException e) {
            status = FMIStatus.FAILED_IO;
            mLogger.e("IOException: " + e);
        }
        mLogger.exit("close");
        return status;
    }

    public ArrayList<FMIBlock> getBlocks() {
        return mBlocks;
    }

    private FMIStatus readHeader() throws IOException {
        mLogger.enter("readHeader");
        FMIStatus status = FMIStatus.OK;
        mFile.seek(0);

        if (mFile.length() >= Constants.TOTAL_HEADER_SIZE) {
            // FMI signature
            boolean signatureMatch = ((mFile.readByte() == Constants.SIGNATURE[0]) &&
                    (mFile.readByte() == Constants.SIGNATURE[1]) &&
                    (mFile.readByte() == Constants.SIGNATURE[2]) &&
                    (mFile.readByte() == Constants.SIGNATURE[3]));
            if (signatureMatch) {
                mHeader.size = mFile.readByte() & 0xFF |
                        (mFile.readByte() & 0xFF) << 8 |
                        (mFile.readByte() & 0xFF) << 16 |
                        (mFile.readByte() & 0xFF) << 24;
                mHeader.majorVersion = mFile.readByte() & 0xFF |
                        (mFile.readByte() & 0xFF) << 8;
                mHeader.minorVersion = mFile.readByte() & 0xFF |
                        (mFile.readByte() & 0xFF) << 8;

                mLogger.d("mHeader.size: " + mHeader.size);
                mLogger.d("mHeader.majorVersion: " + mHeader.majorVersion);
                mLogger.d("mHeader.minorVersion: " + mHeader.minorVersion);
            } else {
                status = FMIStatus.FAILED_IO;
                mLogger.w("Invalid signature");
            }
        }
        mLogger.enter("readHeader");
        return status;
    }

    private FMIStatus writeHeader() throws IOException {
        mLogger.enter("writeHeader");
        FMIStatus status = FMIStatus.OK;
        mFile.seek(0);
        mFile.write(Constants.SIGNATURE);
        mFile.writeInt(LE(Constants.HEADER_SIZE));
        mFile.writeShort(LE((short) Constants.VERSION_MINOR));
        mFile.writeShort(LE((short) Constants.VERSION_MAJOR));
        mLogger.enter("writeHeader");
        return status;
    }

    private FMIStatus readBlocks() throws IOException {
        mLogger.enter("readBlocks");
        FMIStatus status = FMIStatus.OK;
        int offset = Constants.TOTAL_HEADER_SIZE;
        mFile.seek(offset);

        byte[] b = new byte[Constants.TOTAL_BLOCK_HEADER_SIZE];
        while (mFile.read(b) != -1) {
            FMIBlock block = new FMIBlock();
            block.type.format = FMIBlockType.Format.fromValue(b[0] & 0xFF | (b[1] & 0xFF) << 8);
            block.type.id = FMIBlockType.Id.fromValue(b[2] & 0xFF | (b[3] & 0xFF) << 8);
            block.size = b[4] & 0xFF | (b[5] & 0xFF) << 8 | (b[6] & 0xFF) << 16 | (b[7] & 0xFF) << 24;
            offset += Constants.TOTAL_BLOCK_HEADER_SIZE;
            block.offset = offset;
            offset += block.size;
            mFile.seek(offset);
            mBlocks.add(block);
            mLogger.d("Block.format: " + block.type.format.getValue());
            mLogger.d("Block.id: " + block.type.id.getValue());
            mLogger.d("Block.size: " + block.size);
            mLogger.d("Block.offset: " + block.offset);
        }

        mLogger.enter("readBlocks");
        return status;
    }

    private short LE(short v) {
        return (short) (((v << 8) & 0XFF00) |
                ((v >> 8) & 0X00FF));
    }

    private int LE(int v) {
        return (((v << 24) & 0XFF000000) |
                ((v << 8) & 0X00FF0000) |
                ((v >> 8) & 0X0000FF00) |
                ((v >> 24) & 0X000000FF));
    }
}
