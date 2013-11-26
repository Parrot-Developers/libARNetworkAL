package com.parrot.arsdk.arnetworkal;

import android.R.integer;
import android.util.Log;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothGattCharacteristic;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import com.parrot.arsdk.arnetworkal.ARNETWORKAL_ERROR_ENUM;
import com.parrot.arsdk.arnetworkal.ARNETWORKAL_MANAGER_RETURN_ENUM;

public class ARNetworkALBLENetwork
{
    private static String TAG = "ARNetworkALBLENetwork";
    
    private static String ARNETWORKAL_BLENETWORK_PARROT_SERVICE_PREFIX_UUID = "0000f";
    
    private native static void nativeJNIInit();
    
    private ARNetworkALBLEManager bleManager;
    
    private BluetoothGattService recvService;
    private BluetoothGattService sendService;
    private ArrayList<BluetoothGattCharacteristic> array;
    
    static
    {
        //Log.d(TAG, "run static block");
        
        nativeJNIInit();
    }
    
    public ARNetworkALBLENetwork()
    {
        //Log.d(TAG, "new ARNetworkALBLENetwork ");
        
        this.bleManager = null;
        this.array = new ArrayList<BluetoothGattCharacteristic>();
    }
    
    public int connect (ARNetworkALBLEManager bleManager, BluetoothDevice deviceBLEService)
    {
        Log.d(TAG, "connect");
        
        ARNETWORKAL_ERROR_ENUM result = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK;
        BluetoothGattService senderService = null;
        BluetoothGattService receiverService = null;
        
        this.bleManager = bleManager;
        
        /* connect to the device */
        boolean isConnected = bleManager.connect(deviceBLEService);
        if (isConnected == false)
        {
            result = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_BLE_CONNECTION;
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
                if (gattService.getUuid().toString().startsWith(ARNETWORKAL_BLENETWORK_PARROT_SERVICE_PREFIX_UUID))
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
            Log.d(TAG, "senderService: " + senderService.getUuid());
            Log.d(TAG, "receiverService: " + receiverService.getUuid());
            
            this.recvService = receiverService;
            this.sendService = senderService;
            
            /* Registered notification service for receiver */
            /*
            for (BluetoothGattCharacteristic gattCharacteristic : receiverService.getCharacteristics())
            {
                if ((gattCharacteristic.getProperties() & BluetoothGattCharacteristic.PROPERTY_NOTIFY) == BluetoothGattCharacteristic.PROPERTY_NOTIFY)
                {
                    bleManager.setCharacteristicNotification (receiverService, gattCharacteristic);
                }
            }
            */
            
            //TODO: sup
            //temporary just Registered notification service for the 4 Characteristics need by the DELOS
            bleManager.setCharacteristicNotification (this.recvService, receiverService.getCharacteristics().get(14));
            bleManager.setCharacteristicNotification (this.recvService, receiverService.getCharacteristics().get(15));
            bleManager.setCharacteristicNotification (this.recvService, receiverService.getCharacteristics().get(27));
            bleManager.setCharacteristicNotification (this.recvService, receiverService.getCharacteristics().get(28));
            //temporary just Registered notification service for the 4 Characteristics need by the DELOS
            //TODO: sup
            
        }
        
        return result.getValue();
    }
    
    public void disconnect ()
    {
        Log.d(TAG, "disconnect");
        
        bleManager.disconnect();
        bleManager = null;
        
    }
    
    private void unlock ()
    {
        Log.d(TAG, "unlock");
        
        bleManager.unlock ();
        
    }
    
    private int receive ()
    {
        ARNETWORKAL_MANAGER_RETURN_ENUM result = ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_DEFAULT;
        
        if (!bleManager.readData(array))
        {
            result = ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_NO_DATA_AVAILABLE;
        }
        
        return result.getValue();
    }
    
    private DataPop popFrame ()
    {
        DataPop dataPop = new DataPop();
        
        ARNETWORKAL_MANAGER_RETURN_ENUM result = ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_DEFAULT;
        BluetoothGattCharacteristic characteristic = null;
        
        /* -- get a Frame of the receiving buffer -- */
        /* if the receiving buffer not contain enough data for the frame head */
        if (array.size() == 0)
        {
            result = ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_BUFFER_EMPTY;
        }
        
        if (result == ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_DEFAULT)
        {
            characteristic = array.get (0);
            if (characteristic.getValue().length == 0)
            {
                result = ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_BAD_FRAME;
            }
        }
        
        if (result == ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_DEFAULT)
        {
            byte[] currentFrame = characteristic.getValue();
            
            /* get id */
            String frameIdString = characteristic.getUuid().toString().substring(4, 8);
            int frameId = Integer.parseInt(frameIdString, 16);
            
            if (result == ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_DEFAULT)
            {
                /* Get the frame from the buffer */
                dataPop.setId(frameId);
                dataPop.setData(currentFrame);
            }
        }
        
        if (result != ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_BUFFER_EMPTY)
        {
            array.remove (0);
        }
        
        dataPop.setResult (result.getValue());
        return dataPop;
    }
    
    private int pushFrame (int type, int id, int seq, int size, byte[] byteData)
    {
        ARNETWORKAL_MANAGER_RETURN_ENUM result = ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_DEFAULT;
        
        if (result == ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_DEFAULT)
        {
            byte[] data = new byte[byteData.length + 2];
            
            /* Add frame type */
            data[0] = (byte) type;
            
            /* Add frame seq */
            data[1] = (byte) seq;
            
            /* Add frame data */
            System.arraycopy(byteData, 0, data, 2, byteData.length);
            
            /* Get the good characteristic */
            BluetoothGattCharacteristic characteristicToSend = null;
            if (type == ARNETWORKAL_FRAME_TYPE_ENUM.ARNETWORKAL_FRAME_TYPE_ACK.getValue())
            {
                characteristicToSend = recvService.getCharacteristics().get(id);
            }
            else
            {
                characteristicToSend = sendService.getCharacteristics().get(id);
            }
            
            /* write the data */
            if (!bleManager.writeData(data, characteristicToSend))
            {
                result = ARNETWORKAL_MANAGER_RETURN_ENUM.ARNETWORKAL_MANAGER_RETURN_BAD_FRAME;
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
}

