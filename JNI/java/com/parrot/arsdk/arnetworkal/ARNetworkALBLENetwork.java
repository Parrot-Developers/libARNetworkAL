/*
    Copyright (C) 2014 Parrot SA

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the 
      distribution.
    * Neither the name of Parrot nor the names
      of its contributors may be used to endorse or promote products
      derived from this software without specific prior written
      permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
    OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
    OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/
package com.parrot.arsdk.arnetworkal;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Semaphore;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.content.Context;
import android.os.SystemClock;

import com.parrot.arsdk.arsal.ARSALBLEManager;
import com.parrot.arsdk.arsal.ARSALBLEManager.ARSALManagerNotificationData;
import com.parrot.arsdk.arsal.ARSALBLEManagerListener;
import com.parrot.arsdk.arsal.ARSALPrint;
import com.parrot.arsdk.arsal.ARSAL_ERROR_ENUM;
import com.parrot.arsdk.arsal.ARUUID;

public class ARNetworkALBLENetwork implements ARSALBLEManagerListener
{
    private static String TAG = "ARNetworkALBLENetwork";
    private static final long BASIC_TEST_SLEEP = 2000;
    
    private static String ARNETWORKAL_BLENETWORK_NOTIFICATIONS_KEY = "ARNETWORKAL_BLENETWORK_NOTIFICATIONS_KEY";
    private static String ARNETWORKAL_BLENETWORK_PARROT_SERVICE_PREFIX_UUID = "f";
    private static String ARNETWORKAL_BLENETWORK_PARROT_CHARACTERISTIC_PREFIX_UUID_FTP_21 = "fd23";
    private static String ARNETWORKAL_BLENETWORK_PARROT_CHARACTERISTIC_PREFIX_UUID_FTP_51 = "fd53";
    
    public final static String ARNETWORKAL_BLENETWORK_PARROT_SERVICE_PREFIX_UUID_RFCOMM = "fe00";
    public final static String ARNETWORKAL_BLENETWORK_PARROT_CHARACTERISTIC_PREFIX_UUID_RFCOMM_WRITE = "fe01";
    public final static String ARNETWORKAL_BLENETWORK_PARROT_CHARACTERISTIC_PREFIX_UUID_RFCOMM_READ = "fe02";
    
    public static int ARNETWORKAL_BLENETWORK_MEDIA_MTU = 0;
    public static int ARNETWORKAL_BLENETWORK_HEADER_SIZE = 0;
    
    private static int ARNETWORKAL_BLENETWORK_BW_PROGRESS_EACH_SEC = 1;
    private static int ARNETWORKAL_BLENETWORK_BW_NB_ELEMS = 10;
    
    private native static void nativeJNIInit();
    private native static int nativeGetMediaMTU ();
    private native static int nativeGetHeaderSize();
    
    private native static void nativeJNIOnDisconect (long jniARNetworkALBLENetwork);
    private ARSALBLEManager bleManager;
    
    private BluetoothDevice deviceBLEService;
    
    private BluetoothGattService recvService;
    private BluetoothGattService sendService;
    //private ArrayList<BluetoothGattCharacteristic> recvArray;
    private ArrayList<ARSALManagerNotificationData> recvArray;
    
    private int[] bwElementUp;
    private int[] bwElementDown;
    private int bwIndex;
    private Semaphore bwSem;
    private Semaphore bwThreadRunning;
    private int bwCurrentUp;
    private int bwCurrentDown;
    
    private long jniARNetworkALBLENetwork;
    
    static
    {
        ARNETWORKAL_BLENETWORK_MEDIA_MTU = nativeGetMediaMTU ();
        ARNETWORKAL_BLENETWORK_HEADER_SIZE = nativeGetHeaderSize();
        nativeJNIInit();
    }
    
    public ARNetworkALBLENetwork (long jniARNetworkALBLENetwork, Context context)
    {
        this.bleManager = ARSALBLEManager.getInstance(context);
        this.recvArray = new ArrayList<ARSALManagerNotificationData>();
        this.jniARNetworkALBLENetwork = jniARNetworkALBLENetwork;
        
        this.bwElementUp = new int[ARNETWORKAL_BLENETWORK_BW_NB_ELEMS];
        this.bwElementDown = new int[ARNETWORKAL_BLENETWORK_BW_NB_ELEMS];
        this.bwSem = new Semaphore (0);
        this.bwThreadRunning = new Semaphore (0);
    }
    
    public int connect (BluetoothDevice deviceBLEService, int[] notificationIDArray)
    {
        ARNETWORKAL_ERROR_ENUM result = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK;
        BluetoothGattService senderService = null;
        BluetoothGattService receiverService = null;
        
        if (deviceBLEService == null)
        {
            result = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_BAD_PARAMETER;
        }
        
        if (result == ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK)
        {
            SystemClock.sleep(BASIC_TEST_SLEEP);
            ARSAL_ERROR_ENUM resultAL = bleManager.connect(deviceBLEService);
            
            if (resultAL == ARSAL_ERROR_ENUM.ARSAL_OK)
            {
                result = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK;
            }
            else if (resultAL == ARSAL_ERROR_ENUM.ARSAL_ERROR_BLE_STACK)
            {
                result = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_BLE_STACK;
            }
            else
            {
                result = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_BLE_CONNECTION;
            }
        }
        
        if (result == ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK)
        {
            SystemClock.sleep(BASIC_TEST_SLEEP);
            ARSAL_ERROR_ENUM resultAL = bleManager.discoverBLENetworkServices();
            result = (resultAL == ARSAL_ERROR_ENUM.ARSAL_OK) ? ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK : ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_BLE_SERVICES_DISCOVERING;
        }
        
        /* look for the receiver service and the sender service */
        if (result == ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK)
        {
            SystemClock.sleep(BASIC_TEST_SLEEP);

            BluetoothGatt gatt = bleManager.getGatt ();

            if(gatt != null)
            {
                List<BluetoothGattService> serviesArray = bleManager.getServices();
                
                if(serviesArray != null)
                {
                    for (int index = 0 ; index < serviesArray.size() && ((senderService == null) || (receiverService == null)) ; index++ )
                    {
                        BluetoothGattService gattService = serviesArray.get(index);
                        
                        /* check if it is a parrot service */
                        if (ARUUID.getShortUuid(gattService.getUuid()).startsWith(ARNETWORKAL_BLENETWORK_PARROT_SERVICE_PREFIX_UUID))
                        {
                            /* if there is any characteristic */
                            if (gattService.getCharacteristics().size() > 0)
                            {
                                BluetoothGattCharacteristic gattCharacteristic = gattService.getCharacteristics().get(0);
                                
                                if ((senderService == null) && ((gattCharacteristic.getProperties() & BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE) == BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE))
                                {
                                    senderService = gattService;
                                }
                                
                                if ((receiverService == null) && ((gattCharacteristic.getProperties() & BluetoothGattCharacteristic.PROPERTY_NOTIFY) == BluetoothGattCharacteristic.PROPERTY_NOTIFY))
                                {
                                    receiverService = gattService;
                                }
                            }
                        }
                        /*
                         * NO ELSE
                         * It's not a Parrot characteristic, ignore it
                         */
                    }
                }
                else
                {
                    ARSALPrint.e(TAG, "no service");
                }
                
                if ((senderService == null) || (receiverService == null))
                {
                    result = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_BLE_SERVICES_DISCOVERING;
                }
            }
            else
            {
                result = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_BLE_NOT_CONNECTED;
            }
        }
        
        if (result == ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK)
        {
            ARSALPrint.d(TAG, "senderService: " + senderService.getUuid());
            ARSALPrint.d(TAG, "receiverService: " + receiverService.getUuid());
            
            /*
            bwIndex = 0;
            bwCurrentUp = 0;
            bwCurrentDown = 0;
            for (int i = 0 ; i < ARNETWORKAL_BLENETWORK_BW_NB_ELEMS ; i++)
            {
                bwElementUp[i] = 0;
                bwElementDown[i] = 0;
            }
            bwThreadRunning.release();
            */
            
            this.deviceBLEService = deviceBLEService;
            this.recvService = receiverService;
            this.sendService = senderService;
            
            this.bleManager.setListener(this);
            
            List<BluetoothGattCharacteristic> notificationCharacteristics = null;
            if (notificationIDArray != null)
            {
                notificationCharacteristics = new ArrayList<BluetoothGattCharacteristic>();
                /* Add the characteristics to be notified */
                for (int id : notificationIDArray)
                {
                    if(id < receiverService.getCharacteristics().size())
                    {
                        notificationCharacteristics.add(receiverService.getCharacteristics().get(id));
                    }
                    else
                    {
                        ARSALPrint.e(TAG, "error receiverService.getCharacteristics().size(): " + receiverService.getCharacteristics().size() +" id to notify: " + id);
                    }
                }
            }
            else
            {
                notificationCharacteristics = receiverService.getCharacteristics();
            }
            
            /* Notify the characteristics */
            ARSAL_ERROR_ENUM setNotifCharacteristicResult = ARSAL_ERROR_ENUM.ARSAL_OK; //TODO see
            for (BluetoothGattCharacteristic gattCharacteristic : notificationCharacteristics)
            {
                /* if the characteristic can be notified */
                if ((gattCharacteristic.getProperties() & BluetoothGattCharacteristic.PROPERTY_NOTIFY) == BluetoothGattCharacteristic.PROPERTY_NOTIFY)
                {
                    setNotifCharacteristicResult = bleManager.setCharacteristicNotification (receiverService, gattCharacteristic);
                }
                
                switch (setNotifCharacteristicResult)
                {
                    case ARSAL_OK:
                        /* notification successfully set */
                        /* do nothing */
                        break;
                        
                    case ARSAL_ERROR_BLE_CHARACTERISTIC_CONFIGURING:
                        /* This service is unknown by ARNetworkAL*/
                        /* do nothing */
                        break;
                        
                    case ARSAL_ERROR_BLE_NOT_CONNECTED:
                        /* the peripheral is disconnected */
                        result = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_BLE_CONNECTION;
                        break;
                        
                    default:
                        ARSALPrint.e (TAG, "error " + setNotifCharacteristicResult + " unexpected :  " + setNotifCharacteristicResult);
                        break;
                }
            }
            
            bleManager.registerNotificationCharacteristics(notificationCharacteristics, ARNETWORKAL_BLENETWORK_NOTIFICATIONS_KEY);
        }
        
        if (result == ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK)
        {
            ARSAL_ERROR_ENUM resultSal = ARSAL_ERROR_ENUM.ARSAL_OK;
            List<BluetoothGattService> serviesArray = bleManager.getServices();
            
            for (int index=0; ((index < serviesArray.size()) && (resultSal == ARSAL_ERROR_ENUM.ARSAL_OK)); index++)
            {
                BluetoothGattService gattService = serviesArray.get(index);
                List<BluetoothGattCharacteristic> characteristicsArray = gattService.getCharacteristics();
                ARSALPrint.w(TAG, "ARNetwork service " + ARUUID.getShortUuid(gattService.getUuid()));
                
                for (int j=0; ((j < characteristicsArray.size()) && (resultSal == ARSAL_ERROR_ENUM.ARSAL_OK)); j++)
                {
                    BluetoothGattCharacteristic gattCharacteristic = characteristicsArray.get(j);
                    ARSALPrint.w(TAG, "ARNetwork service " + ARUUID.getShortUuid(gattCharacteristic.getUuid()));
                    
                    if (ARUUID.getShortUuid(gattCharacteristic.getUuid()).startsWith(ARNETWORKAL_BLENETWORK_PARROT_CHARACTERISTIC_PREFIX_UUID_FTP_21)
                        || ARUUID.getShortUuid(gattCharacteristic.getUuid()).startsWith(ARNETWORKAL_BLENETWORK_PARROT_CHARACTERISTIC_PREFIX_UUID_FTP_51)
                        || ARUUID.getShortUuid(gattCharacteristic.getUuid()).startsWith(ARNETWORKAL_BLENETWORK_PARROT_CHARACTERISTIC_PREFIX_UUID_RFCOMM_READ))
                    {
                        resultSal = bleManager.setCharacteristicNotification(gattService, gattCharacteristic);
                        ARSALPrint.w(TAG, "ARNetwork ====setCharacteristicNotification " + ARUUID.getShortUuid(gattService.getUuid()) + " " + resultSal);
                    }
                }
            }
        }
        
        return result.getValue();
    }
    
    public void cancel ()
    {
        disconnect ();
        
        /* reset the BLEManager for a new use */
        bleManager.reset();
    }
    
    public void disconnect ()
    {
        synchronized (this)
        {
            //if(deviceBLEService != null)
            //{
                ARSALPrint.d(TAG, "disconnect");
                
                // TODO see not work if bw is not yet initilaized 
                /*
                bwSem.release();
                try
                {
                    bwThreadRunning.acquire ();
                }
                catch (InterruptedException e)
                {
                    e.printStackTrace();
                }*/
                
                bleManager.disconnect();
                
                //cleanup
                cleanup();
                bleManager.setListener(null);
                
            //}
        }
    }
    
    private void cleanup()
    {
        // cleanup the ble references
        deviceBLEService = null;
        recvService = null;
        sendService = null;
        
        recvArray.clear();
    }
    
    private void unlock ()
    {
        ARSALPrint.d(TAG, "unlock");
        
        bleManager.unlock ();
    }
    
    private int receive ()
    {
        ARNETWORKAL_MANAGER_RETURN_ENUM result = ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_DEFAULT;
        
        //if (!bleManager.readData(recvArray))
        if (!bleManager.readDataNotificationData(recvArray, Integer.MAX_VALUE, ARNETWORKAL_BLENETWORK_NOTIFICATIONS_KEY))
        {
            result = ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_NO_DATA_AVAILABLE;
        }
        
        return result.getValue();
    }
    
    private DataPop popFrame ()
    {
        DataPop dataPop = new DataPop();
        
        ARNETWORKAL_MANAGER_RETURN_ENUM result = ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_DEFAULT;
        ARSALManagerNotificationData notification = null;
        
        /* -- get a Frame of the receiving buffer -- */
        /* if the receiving buffer not contain enough data for the frame head */
        if (recvArray.size() == 0)
        {
            result = ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_BUFFER_EMPTY;
        }
        
        if (result == ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_DEFAULT)
        {
            notification = recvArray.get (0);
            if (notification.value.length == 0)
            {
                result = ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_BAD_FRAME;
            }
        }
        
        if (result == ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_DEFAULT)
        {
            byte[] currentFrame = notification.value;
            
            /* get id */
            String frameIdString = ARUUID.getShortUuid(notification.characteristic.getUuid());
            int frameId = Integer.parseInt(frameIdString, 16);
            
            if (result == ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_DEFAULT)
            {
                /* Get the frame from the buffer */
                dataPop.setId(frameId);
                dataPop.setData(currentFrame);
                
                bwCurrentDown += currentFrame.length;
            }
        }
        
        if (result != ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_BUFFER_EMPTY)
        {
            recvArray.remove (0);
        }
        
        dataPop.setResult (result.getValue());
        return dataPop;
    }
    
    private int pushFrame (int type, int id, int seq, int size, byte[] byteData)
    {
        ARNETWORKAL_MANAGER_RETURN_ENUM result = ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_DEFAULT;
        
        if (result == ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_DEFAULT)
        {
            /* The size of byteData is checked before by the JNI */
            byte[] data = new byte[byteData.length + ARNETWORKAL_BLENETWORK_HEADER_SIZE];
            
            /* Add frame type */
            data[0] = (byte) type;
            
            /* Add frame seq */
            data[1] = (byte) seq;
            
            /* Add frame data */
            System.arraycopy(byteData, 0, data, 2, byteData.length);
            
            /* Get the good characteristic */
            BluetoothGattCharacteristic characteristicToSend = null;

            if(sendService != null)
            {
                if(id >= 0 && id < sendService.getCharacteristics().size())
                {
                   characteristicToSend = sendService.getCharacteristics().get(id);
                
                    /* write the data */
                    if (!bleManager.writeData(data, characteristicToSend))
                    {
                        result = ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_BAD_FRAME;
                    }
                    else
                    {
                        bwCurrentUp += data.length;
                    } 
                }
                else
                {
                    result = ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_BAD_PARAMETERS;
                }
                
            }
            else
            {
                result = ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_NETWORK_ERROR;
            }
            
        }
        
        return result.getValue();
    }
    
    private class DataPop
    {
        int id;
        byte[] data;
        int result;
        
        DataPop()
        {
            this.id = 0;
            this.data = null;
            this.result = ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_DEFAULT.getValue();
        }
        
        void setData (byte[] data)
        {
            this.data = data;
        }
        
        void setId (int id)
        {
            this.id = id;
        }
        
        void setResult (int result)
        {
            this.result = result;
        }
        
        byte[] getData ()
        {
            return data;
        }
        
        int getId ()
        {
            return id;
        }
        
        int getResult ()
        {
            return result;
        }
        
    }
    
    public void onBLEDisconnect ()
    {
        nativeJNIOnDisconect (jniARNetworkALBLENetwork);
        
        // cleanup
        cleanup();
    }
}

