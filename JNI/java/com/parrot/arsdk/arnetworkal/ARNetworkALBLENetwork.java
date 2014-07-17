package com.parrot.arsdk.arnetworkal;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Semaphore;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.content.Context;

import com.parrot.arsdk.arsal.ARSALBLEManager;
import com.parrot.arsdk.arsal.ARSALBLEManager.ARSALManagerNotificationData;
import com.parrot.arsdk.arsal.ARSALBLEManagerListener;
import com.parrot.arsdk.arsal.ARSALPrint;
import com.parrot.arsdk.arsal.ARSAL_ERROR_ENUM;
import com.parrot.arsdk.arsal.ARUUID;

public class ARNetworkALBLENetwork implements ARSALBLEManagerListener
{
    private static String TAG = "ARNetworkALBLENetwork";
    
    private static String ARNETWORKAL_BLENETWORK_NOTIFICATIONS_KEY = "ARNETWORKAL_BLENETWORK_NOTIFICATIONS_KEY";
    private static String ARNETWORKAL_BLENETWORK_PARROT_SERVICE_PREFIX_UUID = "f";
    private static String ARNETWORKAL_BLENETWORK_PARROT_CHARACTERISTIC_PREFIX_UUID_FTP_21 = "fd23";
    private static String ARNETWORKAL_BLENETWORK_PARROT_CHARACTERISTIC_PREFIX_UUID_FTP_51 = "fd53";    
    private static int ARNETWORKAL_BLENETWORK_MEDIA_MTU = 0;
    private static int ARNETWORKAL_BLENETWORK_HEADER_SIZE = 0;
    
    private static int ARNETWORKAL_BLENETWORK_BW_PROGRESS_EACH_SEC = 1;
    private static int ARNETWORKAL_BLENETWORK_BW_NB_ELEMS = 10;
    
    private native static void nativeJNIInit();
    private native static int nativeGetMediaMTU ();
    private native static int nativeGetHeaderSize();
    
    private native static void nativeJNIOnDisconect (int jniARNetworkALBLENetwork);
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
    
    private int jniARNetworkALBLENetwork;
    
    static
    {
        ARNETWORKAL_BLENETWORK_MEDIA_MTU = nativeGetMediaMTU ();
        ARNETWORKAL_BLENETWORK_HEADER_SIZE = nativeGetHeaderSize();
        nativeJNIInit();
    }
    
    public ARNetworkALBLENetwork (int jniARNetworkALBLENetwork, Context context)
    {
        this.bleManager = ARSALBLEManager.getInstance(context);
        //this.recvArray = new ArrayList<BluetoothGattCharacteristic>();
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
            result = (bleManager.connect(deviceBLEService) == ARSAL_ERROR_ENUM.ARSAL_OK) ? ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK : ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_BLE_CONNECTION;
        }
        
        if (result == ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK)
        {
            result = (bleManager.discoverBLENetworkServices() == ARSAL_ERROR_ENUM.ARSAL_OK) ? ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK : ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_BLE_SERVICES_DISCOVERING;
        }
        
        /* look for the receiver service and the sender service */
        if (result == ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK)
        {
            BluetoothGatt gatt = bleManager.getGatt ();
            List<BluetoothGattService> serviesArray = gatt.getServices();
            
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
            
            if ((senderService == null) || (receiverService == null))
            {
                result = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_BLE_SERVICES_DISCOVERING;
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
                    notificationCharacteristics.add(receiverService.getCharacteristics().get(id));
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
            BluetoothGatt gatt = bleManager.getGatt ();
            List<BluetoothGattService> serviesArray = gatt.getServices();
            
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
                        || ARUUID.getShortUuid(gattCharacteristic.getUuid()).startsWith(ARNETWORKAL_BLENETWORK_PARROT_CHARACTERISTIC_PREFIX_UUID_FTP_51))
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

