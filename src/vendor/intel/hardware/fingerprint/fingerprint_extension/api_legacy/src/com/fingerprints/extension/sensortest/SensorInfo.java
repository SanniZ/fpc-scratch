/**
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.sensortest;

import android.os.Parcel;
import android.os.Parcelable;

import com.fingerprints.extension.util.Logger;

import java.util.Arrays;
import java.util.HashMap;

public class SensorInfo implements Parcelable {
    public static final String HARDWARE_ID = "hardware_id";
    public static final String LOT_ID = "lot_id";
    public static final String WAFER_ID = "wafer_id";
    public static final String WAFER_POSITION_X = "wafer_position_x";
    public static final String WAFER_POSITION_Y = "wafer_position_y";
    public static final String PRODUCTION_TIMESTAMP = "production_timestamp";
    public static final String COMPANION_CHIP_HARDWARE_ID = "companion_chip_hardware_id";
    public static final String COMPANION_CHIP_LOT_ID = "companion_chip_lot_id";
    public static final String VENDOR_DATA = "vendor_data";
    public static final String TOTAL_NUM_OTP_BIT_ERRORS = "total_num_otp_bit_errors";
    public static final String MAX_NUM_OTP_BIT_ERRORS_IN_BYTE = "max_num_otp_bit_errors_in_byte";
    public static final String PRODUCT_TYPE = "product_type";

    /* These flags needs to be in sync with corresponding flags in fpc_hw_identification_types.h */
    public static final int FPC_SENSOR_DIE_HARDWARE_ID_FIELD  = (1<<0);
    public static final int FPC_SENSOR_DIE_LOT_ID_FIELD = (1<<1);
    public static final int FPC_SENSOR_DIE_WAFER_ID_FIELD = (1<<2);
    public static final int FPC_SENSOR_DIE_WAFER_POSITION_X_FIELD = (1<<3);
    public static final int FPC_SENSOR_DIE_WAFER_POSITION_Y_FIELD = (1<<4);
    public static final int FPC_SENSOR_DIE_PRODUCTION_TIMESTAMP_FIELD = (1<<5);

    public static final int FPC_COMPANION_DIE_HARDWARE_ID_FIELD = (1<<0);
    public static final int FPC_COMPANION_DIE_LOT_ID_FIELD = (1<<1);

    public static final int FPC_VENDOR_HW_DATA = (1<<0);

    public static final String NA_FORMAT_RESULT = "N/A (%s)";

    private Logger mLogger = new Logger(getClass().getSimpleName());
    private int mSensorDieValidFlag;
    private int mHardwareId;
    private String mHardwareIdStr;
    private String mLotId; /* Sensor wafer package id */
    private String mLotIdStr;
    private int mWaferId;
    private String mWaferIdStr;
    private int mWaferPositionX;
    private String mWaferPositionXStr;
    private int mWaferPositionY;
    private String mWaferPositionYStr;
    private String mProductionTimestamp; /* Format yyyy-mm-dd */
    private String mProductionTimestampStr; /* Format yyyy-mm-dd */
    private int mCompanionDieValidFlag;
    private int mCompanionChipHardwareId;
    private String mCompanionChipHardwareIdStr;
    private String mCompanionChipLotId; /* Wafer package id */
    private String mCompanionChipLotIdStr; /* Wafer package id */
    private int mVendorHwValidFlag;
    private byte[] mVendorData;
    private String mVendorDataStr;
    private int mTotalNumOtpBitErrors;
    private String mTotalNumOtpBitErrorsStr;
    private int mMaxNumOtpBitErrorsInByte;
    private String mMaxNumOtpBitErrorsInByteStr;
    private int mProductType;

    private HashMap<String, Object> mParameterMap;

    public SensorInfo() {
    }

    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("\nHardwareId: ");
        sb.append(mHardwareIdStr);
        sb.append("\nLotId: ");
        sb.append(mLotIdStr);
        sb.append("\nWaferId: ");
        sb.append(mWaferIdStr);
        sb.append("\nWaferPositionX: ");
        sb.append(mWaferPositionXStr);
        sb.append("\nWaferPositionY: ");
        sb.append(mWaferPositionYStr);
        sb.append("\nProductionTimestamp: ");
        sb.append(mProductionTimestampStr);
        sb.append("\nCompanionChipHardwareId: ");
        sb.append(mCompanionChipHardwareIdStr);
        sb.append("\nCompanionChipLotId: ");
        sb.append(mCompanionChipLotIdStr);
        sb.append("\nVendorData: ");
        sb.append(mVendorDataStr);
        sb.append("\nTotalNumOtpBitErrors: ");
        sb.append(mTotalNumOtpBitErrorsStr);
        sb.append("\nMaxNumOtpBitErrorsInByte: ");
        sb.append(mMaxNumOtpBitErrorsInByteStr);
        sb.append("\nProductType: " + mProductType);
        return sb.toString();
    }

    public void print() {
        mLogger.i("SensorInfo" + toString());
    }

    private SensorInfo(Parcel in) {
        try {
            mSensorDieValidFlag = in.readInt();
            mHardwareId = in.readInt();
            mLotId = in.readString();
            mWaferId = in.readInt();
            mWaferPositionX = in.readInt();
            mWaferPositionY = in.readInt();
            mProductionTimestamp = in.readString();
            mCompanionDieValidFlag = in.readInt();
            mCompanionChipHardwareId = in.readInt();
            mCompanionChipLotId = in.readString();
            mVendorHwValidFlag = in.readInt();
            mVendorData = in.createByteArray();
            mTotalNumOtpBitErrors = in.readInt();
            mMaxNumOtpBitErrorsInByte = in.readInt();

            mHardwareIdStr = parseValue(mSensorDieValidFlag, FPC_SENSOR_DIE_HARDWARE_ID_FIELD,
                                        "0x" + Integer.toHexString(mHardwareId));
            mLotIdStr = parseValue(mSensorDieValidFlag, FPC_SENSOR_DIE_LOT_ID_FIELD, mLotId);
            mWaferIdStr = parseValue(mSensorDieValidFlag, FPC_SENSOR_DIE_WAFER_ID_FIELD,
                                     Integer.toString(mWaferId));
            mWaferPositionXStr = parseValue(mSensorDieValidFlag,
                                            FPC_SENSOR_DIE_WAFER_POSITION_X_FIELD,
                                            Integer.toString(mWaferPositionX));
            mWaferPositionYStr = parseValue(mSensorDieValidFlag,
                                            FPC_SENSOR_DIE_WAFER_POSITION_Y_FIELD,
                                            Integer.toString(mWaferPositionY));
            mProductionTimestampStr = parseValue(mSensorDieValidFlag,
                                                 FPC_SENSOR_DIE_PRODUCTION_TIMESTAMP_FIELD,
                                                 mProductionTimestamp);
            mCompanionChipHardwareIdStr = parseValue(mCompanionDieValidFlag,
                                                     FPC_COMPANION_DIE_HARDWARE_ID_FIELD,
                                                     "0x" + Integer.toHexString(
                                                      mCompanionChipHardwareId));
            mCompanionChipLotIdStr = parseValue(mCompanionDieValidFlag,
                                                FPC_COMPANION_DIE_LOT_ID_FIELD,
                                                mCompanionChipLotId);
            mVendorDataStr = parseValue(mVendorHwValidFlag, FPC_VENDOR_HW_DATA,
                                        Arrays.toString(mVendorData));
            mTotalNumOtpBitErrorsStr = parseValue(mTotalNumOtpBitErrors >= 0,
                                                  Integer.toString(mTotalNumOtpBitErrors));
            mMaxNumOtpBitErrorsInByteStr = parseValue(mTotalNumOtpBitErrors >= 0,
                                                      Integer.toString(mMaxNumOtpBitErrorsInByte));
            mProductType = in.readInt();
        } catch (Exception e) {
            mLogger.e("Exception: " + e);
        }
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mSensorDieValidFlag);
        dest.writeInt(mHardwareId);
        dest.writeString(mLotId);
        dest.writeInt(mWaferId);
        dest.writeInt(mWaferPositionX);
        dest.writeInt(mWaferPositionY);
        dest.writeString(mProductionTimestamp);
        dest.writeInt(mCompanionDieValidFlag);
        dest.writeInt(mCompanionChipHardwareId);
        dest.writeString(mCompanionChipLotId);
        dest.writeInt(mVendorHwValidFlag);
        dest.writeByteArray(mVendorData);
        dest.writeInt(mTotalNumOtpBitErrors);
        dest.writeInt(mMaxNumOtpBitErrorsInByte);
        dest.writeInt(mProductType);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public static final Parcelable.Creator<SensorInfo> CREATOR =
            new Parcelable.Creator<SensorInfo>() {
                @Override
                public SensorInfo createFromParcel(Parcel in) {
                    return new SensorInfo(in);
                }

                @Override
                public SensorInfo[] newArray(int size) {
                    return new SensorInfo[size];
                }
            };

    public HashMap<String, Object> getParameterMap() {
        if (mParameterMap == null) {
            mParameterMap = new HashMap<>();
            mParameterMap.put(HARDWARE_ID, mHardwareIdStr);
            mParameterMap.put(LOT_ID, mLotIdStr);
            mParameterMap.put(WAFER_ID, mWaferIdStr);
            mParameterMap.put(WAFER_POSITION_X, mWaferPositionXStr);
            mParameterMap.put(WAFER_POSITION_Y, mWaferPositionYStr);
            mParameterMap.put(PRODUCTION_TIMESTAMP, mProductionTimestampStr);
            mParameterMap.put(COMPANION_CHIP_HARDWARE_ID, mCompanionChipHardwareIdStr);
            mParameterMap.put(COMPANION_CHIP_LOT_ID, mCompanionChipLotIdStr);
            mParameterMap.put(VENDOR_DATA, mVendorDataStr);
            mParameterMap.put(TOTAL_NUM_OTP_BIT_ERRORS, mTotalNumOtpBitErrorsStr);
            mParameterMap.put(MAX_NUM_OTP_BIT_ERRORS_IN_BYTE, mMaxNumOtpBitErrorsInByteStr);
            mParameterMap.put(PRODUCT_TYPE, mProductType);
        }
        return mParameterMap;
    }

    public int getHardwareId() {
        return mHardwareId;
    }

    public int getDeviceId() {
        return extractSubBitValue(mHardwareId, 8, 8);
    }

    public int getVarId() {
        return extractSubBitValue(mHardwareId, 4, 4);
    }

    public int getRevision() {
        return extractSubBitValue(mHardwareId, 4, 0);
    }

    public int getProductType() {
        return mProductType;
    }

    public String getHardwareIdHex() {
        return Integer.toHexString(getHardwareId());
    }

    public String getDeviceIdHex() {
        return Integer.toHexString(getDeviceId());
    }

    public String getVarIdHex() {
        return Integer.toHexString(getVarId());
    }

    public String getRevisionHex() {
        return Integer.toHexString(getRevision());
    }

    public String getProductTypeString() {
        return Integer.toString(getProductType());
    }

    private int extractSubBitValue(final int l, final int nrBits, final int offset) {
        final int rightShifted = l >>> offset;
        final int mask = (1 << nrBits) - 1;
        return rightShifted & mask;
    }

    private boolean isFieldSet(int bitField, int mask) {
        if ((bitField & mask) > 0) {
            return true;
        }

        return false;
    }

    private String parseValue(boolean valueAvailable, String value) {
        if (!valueAvailable) {
            return String.format(NA_FORMAT_RESULT, value);
        }

        return value;
    }

    private String parseValue(int bitField, int mask, String value) {
        return parseValue(isFieldSet(bitField, mask), value);
    }
}
