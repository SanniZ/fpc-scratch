/**
 * Copyright (c) 2017 Fingerprint Cards AB <tech@fingerprints.com>
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 */

package com.fingerprints.extension.engineering;

import android.os.Handler;
import android.os.RemoteException;

import com.fingerprints.extension.util.ArrayUtils;
import com.fingerprints.extension.util.Logger;
import com.fingerprints.extension.V1_0.IFingerprintEngineering;
import com.fingerprints.extension.V1_0.IImageInjectionCallback;
import com.fingerprints.extension.V1_0.IImageCaptureCallback;
import com.fingerprints.extension.V1_0.ImageCaptureData;


import java.util.ArrayList;

public class FingerprintEngineering {
    private Logger mLogger = new Logger(getClass().getSimpleName());
    private IFingerprintEngineering mFingerprintEngineering;
    private ImageSubscriptionCallback mImageSubscriptionCallback;
    private ImageInjectionCallback mImageInjectionCallback;
    private Callback<?> mCaptureCallback;
    private SensorSize mSensorSize;
    private SensorImage.BitsPerPixel mBitsPerPixel = SensorImage.BitsPerPixel.BPP_8;
    private Handler mHandler;

    /**
     * ImageType values represent different stages of processing of the image.
     */
    public enum ImageType {
        /**
         * Raw image (not preprocessed)
         */
        RAW,
        /**
         * Preprocessed image
         */
        PREPROCESSED
    }

    public enum CaptureStatus {
        ERROR,
        OK,
    }

    public enum CaptureMode {
        CAPTURE,
        VERIFY,
        ENROLL;
    }

    public enum CacResult {
        CAC_SUCCESS(0),
        CAC_ERROR_GENERAL(1000),
        CAC_ERROR_FINGER_POOR_COVERAGE(1001),
        CAC_ERROR_FINGER_UP_TOO_EARLY(1002),
        CAC_ERROR_CALIBRATION(1003),
        CAC_ERROR_CAL_MAX_ATTEMPTS(1004),
        CAC_ERROR_FINGER_NOT_STABLE(1005),
        CAC_ERROR_NO_FINGER(1006),
        CAC_ERROR_DB_MAX_ATTEMPTS(1007),
        CAC_ERROR_IMAGE_CAPTURE(1008),
        CAC_ERROR_IMAGE_BAD_QUALITY(1009),
        CAC_ERROR_FIFO_UNDERFLOW(1010),
        CAC_ERROR_NOT_A_FINGER(1011),
        CAC_ERROR_MEMORY(1012),
        CAC_ERROR_METADATA(1013),
        CAC_ERROR_UNKNOWN(-1);

        private final int mCacResult;
        CacResult(int cacResult) {
            mCacResult = cacResult;
        }

        public static CacResult fromInteger(int cacValue) {
            for (CacResult cacResult : CacResult.values()) {
                if  (cacResult.mCacResult == cacValue) {
                    return cacResult;
                }
            }
            return CAC_ERROR_UNKNOWN;
        }

        public boolean isFailure() {
            return CAC_SUCCESS != this;
        }
    }

    public enum EnrollResult {
        FPC_ENROL_COMPLETED(0),
        FPC_ENROL_PROGRESS(1),
        FPC_ENROL_FAILED_COULD_NOT_COMPLETE(2),
        FPC_ENROL_FAILED_ALREADY_ENROLED(3),
        FPC_ENROL_IMAGE_LOW_COVERAGE(4),
        FPC_ENROL_IMAGE_TOO_SIMILAR(5),
        FPC_ENROL_IMAGE_LOW_QUALITY(6),

        FPC_ERROR_INPUT(-1),
        FPC_ERROR_TIMEDOUT(-2),
        FPC_ERROR_ALLOC(-3),
        FPC_ERROR_COMM(-4),
        FPC_ERROR_NOSPACE(-5),
        FPC_ERROR_IO(-6),
        FPC_ERROR_CANCELLED(-7),
        FPC_ERROR_NOENTITY(-8),
        FPC_ERROR_HARDWARE(-9),
        FPC_ERROR_CONFIG(-10),
        FPC_ERROR_NOT_INITIALIZED(-11),
        FPC_ERROR_RESET_HARDWARE(-12),
        FPC_ERROR_PN(-13),
        FPC_ERROR_SENSOR_BROKEN(-14),
        FPC_ERROR_TOO_MANY_DEAD_PIXELS(-15),

        FPC_ENROL_IMAGE_UNKNOWN(-100);

        private final int mEnrollResult;
        EnrollResult(int enrollResult) {
            mEnrollResult = enrollResult;
        }

        public static EnrollResult fromInteger(int enrollValue) {
            for (EnrollResult enrollResult : EnrollResult.values()) {
                if  (enrollResult.mEnrollResult == enrollValue) {
                    return enrollResult;
                }
            }
            return FPC_ENROL_IMAGE_UNKNOWN;
        }

        public boolean isSuccess() {
            return mEnrollResult >= 0 && mEnrollResult != FPC_ENROL_FAILED_COULD_NOT_COMPLETE.mEnrollResult
                    && mEnrollResult != FPC_ENROL_FAILED_ALREADY_ENROLED.mEnrollResult;
        }

        public boolean isAccepted() {
            if (!isSuccess()) {
                return false;
            }
            switch (this) {
                case FPC_ENROL_FAILED_COULD_NOT_COMPLETE:
                case FPC_ENROL_FAILED_ALREADY_ENROLED:
                case FPC_ENROL_IMAGE_LOW_COVERAGE:
                case FPC_ENROL_IMAGE_LOW_QUALITY:
                    return false;
                default:
                    return true;
            }
        }

        public boolean isComplete() {
            return mEnrollResult == FPC_ENROL_COMPLETED.mEnrollResult;
        }

        public boolean isRejected() {
            return !isAccepted() && (mEnrollResult == FPC_ENROL_IMAGE_LOW_QUALITY.mEnrollResult
                    || mEnrollResult == FPC_ENROL_IMAGE_LOW_COVERAGE.mEnrollResult);
        }

        public int getResultCode() {
            return mEnrollResult;
        }
    }

    public class ImageCaptureResult {
        private static final int CAPTURE_RESULT_OK = 0;
        private final int mImageCaptureResult;
        private final CacResult mCacResult;
        public ImageCaptureResult(int imageCaptureResult, int cacResult) {
            mImageCaptureResult = imageCaptureResult;
            mCacResult = CacResult.fromInteger(cacResult);
        }
        public CacResult getCacResult() {
            return mCacResult;
        }
        public int getResultCode() {
            return mImageCaptureResult;
        }
        public boolean isSuccess() {
            return getResultCode() == CAPTURE_RESULT_OK;
        }
    }

    public abstract class ImageData {
        private final SensorImage mRawImage;
        private final SensorImage mEnhancedImage;
        private final ImageCaptureResult mCaptureResult;

        public ImageData(final int captureResult, final int cacResult, final byte[] rawImage,
                         final byte[] enhancedImage) {
            mCaptureResult = new ImageCaptureResult(captureResult, cacResult);
            mRawImage = new SensorImage(mBitsPerPixel, mSensorSize.mWidth, mSensorSize.mHeight, rawImage);
            mEnhancedImage = new SensorImage(mBitsPerPixel, mSensorSize.mWidth, mSensorSize.mHeight, enhancedImage);
        }

        private ImageData(final ImageCaptureData imageCaptureData) {
            this(imageCaptureData.captureResult, imageCaptureData.cacResult,
                    ArrayUtils.toByteArray(imageCaptureData.rawImage),
                    ArrayUtils.toByteArray(imageCaptureData.enhancedImage));
        }

        public SensorImage getRawImage() {
            return mRawImage;
        }

        public SensorImage getEnhancedImage() {
            return mEnhancedImage;
        }

        public ImageCaptureResult getCaptureResult() {
            return mCaptureResult;
        }

        public CaptureStatus getStatus() {
            if (mCaptureResult.getResultCode() == 0) {
                return CaptureStatus.OK;
            } else {
                return CaptureStatus.ERROR;
            }
        }
    }

    public class CaptureData extends ImageData {
        private CaptureData(final ImageCaptureData imageCaptureData) {
            super(imageCaptureData);
        }
    }

    public class EnrollData extends ImageData {
        private final int mUserId;
        private final EnrollResult mEnrollResult;
        private final int mRemaining;

        private EnrollData(final ImageCaptureData imageCaptureData) {
            super(imageCaptureData);
            mEnrollResult = EnrollResult.fromInteger(imageCaptureData.enrollResult);
            mUserId = imageCaptureData.userId;
            mRemaining = imageCaptureData.remainingSamples;
        }

        public int getRemaining() {
            return mRemaining;
        }

        public int getUserId() {
            return mUserId;
        }

        public EnrollResult getEnrollResult() {
            return mEnrollResult;
        }

        public CaptureStatus getStatus() {
            if ((super.getStatus() == CaptureStatus.OK) && (mEnrollResult.isSuccess())) {
                return CaptureStatus.OK;
            } else {
                return CaptureStatus.ERROR;
            }
        }
    }

    public class VerifyData extends ImageData {
        public static final int USER_ID_MATCH_FAILED = 0;
        private final int mIdentifyResult;
        private final int mTemplateUpdateResult;
        private final int mUserId;
        private final int mCoverage;
        private final int mQuality;

        private VerifyData(final ImageCaptureData imageCaptureData) {
            super(imageCaptureData);
            mIdentifyResult = imageCaptureData.identifyResult;
            mTemplateUpdateResult = imageCaptureData.templateUpdateResult;
            mUserId = imageCaptureData.userId;
            mCoverage = imageCaptureData.coverage;
            mQuality = imageCaptureData.quality;
        }

        public int getIdentifyResult() {
            return mIdentifyResult;
        }

        public int getQuality() {
            return mQuality;
        }

        public int getCoverage() {
            return mCoverage;
        }

        public int getTemplateUpdateResult() {
            return mTemplateUpdateResult;
        }

        public int getUserId() {
            return mUserId;
        }

        public CaptureStatus getStatus() {
            if ((super.getStatus() == CaptureStatus.OK) && (mIdentifyResult == 0)) {
                return CaptureStatus.OK;
            } else {
                return CaptureStatus.ERROR;
            }
        }
    }

    private IImageInjectionCallback mIImageInjectionCallback =
            new IImageInjectionCallback.Stub() {
                public ArrayList<Byte> onInject() {
                    mLogger.enter("onInject");
                    byte[] imageData = new byte[0];
                    if (mImageInjectionCallback != null) {
                        ImageInjectionCallback.InjectionError error = null;
                        try {
                            SensorImage image = null;
                            image = mImageInjectionCallback.onInject(mBitsPerPixel,
                                    mSensorSize.mWidth, mSensorSize.mHeight);
                            if (image != null && image.getPixels() != null) {
                                if (image.getBitsPerPixel() != mBitsPerPixel
                                        || image.getWidth() != mSensorSize.mWidth
                                        || image.getHeight() != mSensorSize.mHeight) {
                                    error = ImageInjectionCallback.InjectionError.IMAGE_FORMAT_INCONSISTENT;
                                } else {
                                    imageData = image.getPixels();
                                }
                            } else {
                                error = ImageInjectionCallback.InjectionError.UNSPECIFIED_INJECTION_ERROR;
                            }
                        } catch (Exception e) {
                            error = ImageInjectionCallback.InjectionError.UNSPECIFIED_INJECTION_ERROR;
                            mLogger.e("Exception: " + e);
                        }
                        if (error != null) {
                            mLogger.e("Image injection error occured, code: " + error);
                            onError(error);
                        }
                    }
                    mLogger.exit("onInject");
                    return ArrayUtils.toArrayList(imageData);
                }

                public void onCancel() {
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            if (mImageInjectionCallback != null) {
                                mImageInjectionCallback.onCancel();
                            }
                        }
                    });
                }

                public void onError(final ImageInjectionCallback.InjectionError errorCode) {
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            if (mImageInjectionCallback != null) {
                                mImageInjectionCallback.onError(errorCode);
                            }
                        }
                    });
                }
            };

    public interface ImageInjectionCallback {

        public enum InjectionError {
            /**
             * Unspecified error during image injection
             */
            UNSPECIFIED_INJECTION_ERROR,
            /**
             * image returned from onInject had inconsistent format
             */
            IMAGE_FORMAT_INCONSISTENT,
            /**
             * image returned from onInject did not match the required
             * format
             */
            IMAGE_FORMAT_NOT_SUPPORTED
        }

        public SensorImage onInject(SensorImage.BitsPerPixel bitsPerPixel, int width, int height);

        public void onCancel();

        public void onError(InjectionError errorCode);
    }

    public interface Callback<T> {
        void onResult(final T data);
    }

    public interface ImageSubscriptionCallback {
        public void onImage(final ImageData data);
    }

    private IImageCaptureCallback mIImageSubscriptionCallback = new IImageCaptureCallback.Stub() {
        public void onImage(ImageCaptureData imageCaptureData) {
            if(mImageSubscriptionCallback != null) {
                ImageData data = null;
                switch(CaptureMode.values()[imageCaptureData.mode]) {
                    case CAPTURE: {
                        data = new CaptureData(imageCaptureData);
                        break;
                    }
                    case VERIFY: {
                        data = new VerifyData(imageCaptureData);
                        break;
                    }
                    case ENROLL: {
                        data = new EnrollData(imageCaptureData);
                        break;
                    }
                }
                mImageSubscriptionCallback.onImage(data);
            }
        }
    };

    private IImageCaptureCallback mICaptureCallback = new IImageCaptureCallback.Stub() {
        public void onImage(ImageCaptureData imageCaptureData) {
            Callback callback = mCaptureCallback;
            if (imageCaptureData.remainingSamples == 0) {
                mCaptureCallback = null;
                mLogger.d("onImage callback cleared");
            }

            if(callback != null) {
                switch(CaptureMode.values()[imageCaptureData.mode]) {
                    case CAPTURE: {
                        callback.onResult(new CaptureData(imageCaptureData));
                        break;
                    }
                    case VERIFY: {
                        callback.onResult(new VerifyData(imageCaptureData));
                        break;
                    }
                    case ENROLL: {
                        callback.onResult(new EnrollData(imageCaptureData));
                        break;
                    }
                }
            } else {
                mLogger.e("onImage, callback is null");
            }
        }
    };

    public FingerprintEngineering() throws RemoteException {
        mLogger.enter("FingerprintEngineering");
        mHandler = new Handler();
        mFingerprintEngineering = IFingerprintEngineering.getService();
        if (mFingerprintEngineering == null) {
            throw new RemoteException("Could not get IFingerprintEngineering");
        } else {
            mSensorSize = getSensorSize();
            mLogger.enter("mSensorSize " + mSensorSize.mWidth + " x " + mSensorSize.mHeight);
        }
        mLogger.exit("FingerprintEngineering");
    }

    public SensorSize getSensorSize() {
        mLogger.enter("getSensorSize");
        SensorSize sensorSize = null;
        if (mFingerprintEngineering != null) {
            try {
                sensorSize = new SensorSize(mFingerprintEngineering.getSensorSize());
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("getSensorSize");
        return sensorSize;
    }

    public void startCapture(Callback<CaptureData> callback) {
        mLogger.enter("startCapture");
        mCaptureCallback = callback;
        if (mFingerprintEngineering != null) {
            try {
                mFingerprintEngineering.startCapture(mICaptureCallback, CaptureMode.CAPTURE.ordinal());
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("startCapture");
    }

    public long getEnrollChallenge() {
        mLogger.enter("getEnrollChallenge");
        if (mFingerprintEngineering != null) {
            try {
                return mFingerprintEngineering.getEnrollChallenge();
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("getEnrollChallenge");
        return 0;
    }

    public void setEnrollToken(byte[] token) {
        mLogger.enter("setEnrollToken");
        if (mFingerprintEngineering != null) {
            try {
                mFingerprintEngineering.setEnrollToken(ArrayUtils.toArrayList(token));
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("setEnrollToken");
    }

    public void startVerify(Callback<VerifyData> callback) {
        mLogger.enter("startVerify");
        mCaptureCallback = callback;
        if (mFingerprintEngineering != null) {
            try {
                mFingerprintEngineering.startCapture(mICaptureCallback, CaptureMode.VERIFY.ordinal());
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("startVerify");
    }

    public void startEnroll(Callback<EnrollData> callback) {
        mLogger.enter("startEnroll");
        mCaptureCallback = callback;
        if (mFingerprintEngineering != null) {
            try {
                mFingerprintEngineering.startCapture(mICaptureCallback, CaptureMode.ENROLL.ordinal());
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("startEnroll");
    }

    public void cancelCapture() {
        mLogger.enter("cancelCapture");
        if (mFingerprintEngineering != null) {
            try {
                mFingerprintEngineering.cancelCapture();
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("cancelCapture");
    }

    public void startImageSubscription(ImageSubscriptionCallback callback) {
        mLogger.enter("startImageSubscription");
        mImageSubscriptionCallback = callback;
        if (mFingerprintEngineering != null) {
            try {
                mFingerprintEngineering.startImageSubscription(
                        mIImageSubscriptionCallback);
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("startImageSubscription");
    }

    public void stopImageSubscription() {
        mLogger.enter("stopImageSubscription");
        if (mFingerprintEngineering != null) {
            try {
                mFingerprintEngineering.stopImageSubscription();
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("stopImageSubscription");
    }

    public void startImageInjection(ImageInjectionCallback callback) {
        mLogger.enter("startImageInjection");
        mImageInjectionCallback = callback;
        if (mFingerprintEngineering != null) {
            try {
                mFingerprintEngineering.startImageInjection(mIImageInjectionCallback);
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("startImageInjection");
    }

    public void stopImageInjection() {
        mLogger.enter("stopImageInjection");
        if (mFingerprintEngineering != null) {
            try {
                mFingerprintEngineering.stopImageInjection();
            } catch (RemoteException e) {
                mLogger.e("RemoteException: ", e);
            }
        }
        mLogger.exit("stopImageInjection");
    }
}
