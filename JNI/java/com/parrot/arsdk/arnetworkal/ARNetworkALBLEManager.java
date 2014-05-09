package com.parrot.arsdk.arnetworkal;

import java.util.ArrayList;
import java.util.Dictionary;
import java.util.HashMap;
import java.util.List;
import java.util.UUID;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import android.annotation.TargetApi;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.BluetoothGattDescriptor;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import android.bluetooth.BluetoothAdapter;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import com.parrot.arsdk.arsal.ARSALPrint;

@TargetApi(18)
public class ARNetworkALBLEManager
{
    private static String TAG = "ARNetworkALBLEManager";
    
    private static final UUID ARNETWORKALBLEMANAGER_CHARACTERISTIC_UPDATE_NOTIFICATION_DESCRIPTOR_UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");
    
    private static final int ARNETWORKAL_BLEMANAGER_CONNECTION_TIMEOUT_SEC  = 5;
    
    private ArrayList<ArrayList<BluetoothGattCharacteristic>> mGattCharacteristics;
    
    private Context context;
    private BluetoothDevice deviceBLEService;
    private BluetoothGatt activeGatt;
    
    private ARNetworkALBLEManagerListener listener;
    
    //boolean gattConnected = false;
    
    private List<BluetoothGattCharacteristic> characteristicNotifications;
    
    private Semaphore connectionSem;
    private Semaphore disconnectionSem;
    private Semaphore discoverServicesSem;
    private Semaphore discoverCharacteristicsSem;
    private Semaphore readCharacteristicSem;
    private Semaphore configurationSem;
    
    private Lock readCharacteristicMutex;
    
    private ARNETWORKAL_ERROR_ENUM discoverServicesError;
    private ARNETWORKAL_ERROR_ENUM discoverCharacteristicsError;
    private ARNETWORKAL_ERROR_ENUM configurationCharacteristicError;
    
    private boolean askDisconnection;
    private boolean isDiscoveringServices;
    private boolean isDiscoveringCharacteristics;
    private boolean isConfiguringCharacteristics;
    
    /**
     * Constructor
     */
    public ARNetworkALBLEManager (Context context)
    {
        this.context = context;
        this.deviceBLEService =  null;
        this.activeGatt = null;
        
        characteristicNotifications = new ArrayList<BluetoothGattCharacteristic> ();
        
        listener = null;
        
        connectionSem = new Semaphore (0);
        disconnectionSem = new Semaphore (0);
        discoverServicesSem = new Semaphore (0);
        discoverCharacteristicsSem = new Semaphore (0);
        readCharacteristicSem = new Semaphore (0);
        configurationSem = new Semaphore (0);

        askDisconnection = false;
        isDiscoveringServices = false;
        isDiscoveringCharacteristics = false;
        isConfiguringCharacteristics = false;
        
        discoverServicesError = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK;
        discoverCharacteristicsError = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK;
        configurationCharacteristicError = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK;
        
        readCharacteristicMutex = new ReentrantLock ();
    }
    
    /**
     * Destructor
     */
    public void finalize () throws Throwable
    {
        try
        {
            disconnect ();
        }
        finally
        {
            super.finalize ();
        }
    }
    
    @TargetApi(18)
    public ARNETWORKAL_ERROR_ENUM connect (BluetoothDevice deviceBLEService)
    {
        ARNETWORKAL_ERROR_ENUM result = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK;
        synchronized (this)
        {
            /* if there is an active activeGatt, disconnecting it */
            if (activeGatt != null) 
            {
                disconnect();
            }
            
            /* connection to the new activeGatt */
            this.deviceBLEService = deviceBLEService;
            /*this.activeGatt = */deviceBLEService.connectGatt (context, false, gattCallback);
            
            /* wait the connect semaphore*/
            try
            {
                connectionSem.tryAcquire (ARNETWORKAL_BLEMANAGER_CONNECTION_TIMEOUT_SEC, TimeUnit.SECONDS);
            }
            catch (InterruptedException e)
            {
                e.printStackTrace();
            }
            
            if (activeGatt != null)
            {
                // TODO see
            }
            else
            {
                /* Connection failed */
                result = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_BLE_CONNECTION;
            }
        }
        
        return result;
    }
    
    public void disconnect ()
    {
        synchronized (this)
        {
            ARSALPrint.d(TAG, "disconnect ...");
            if (activeGatt != null)
            {
                askDisconnection = true;
                
                activeGatt.disconnect();
                
                ARSALPrint.d(TAG, "wait the disconnect Semaphore");
                
                /* wait the disconnect Semaphore*/
                try
                {
                    disconnectionSem.acquire ();
                }
                catch (InterruptedException e)
                {
                    e.printStackTrace();
                }
                
                //TODO see removeListener
                
                askDisconnection = false;
                
            }
        }
    }
    
    public ARNETWORKAL_ERROR_ENUM discoverBLENetworkServices ()
    {
        ARNETWORKAL_ERROR_ENUM result = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK;
        synchronized (this)
        {
            /* If there is an active Gatt, disconnect it */
            if (activeGatt != null)
            {
                isDiscoveringServices = true;
                discoverServicesError = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK;
                
                /* run the discovery of the activeGatt services */
                boolean discoveryRes = activeGatt.discoverServices();
                
                if (discoveryRes)
                {
                    /* wait the discoverServices semaphore*/
                    try
                    {
                        discoverServicesSem.acquire ();
                        result = discoverServicesError;
                    }
                    catch (InterruptedException e)
                    {
                        e.printStackTrace();
                        result = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR;
                    }
                }
                else
                {
                    result = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR;
                }
                
                isDiscoveringServices = false;
            }
            else
            {
                result = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_BLE_NOT_CONNECTED;
            }
        }
        
        return result;
    }
    
    public BluetoothGatt getGatt ()
    {
        return activeGatt;
    }
    
    public void setListener(ARNetworkALBLEManagerListener listener)
    {
        this.listener = listener;
    }
    
    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback()
    {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState)
        {
            String intentAction;
            if (newState == BluetoothProfile.STATE_CONNECTED)
            {
                ARSALPrint.d(TAG, "Connected to GATT server.");
                
                activeGatt = gatt;
                
                /* post a connect Semaphore */
                connectionSem.release();
            }
            else if (newState == BluetoothProfile.STATE_DISCONNECTED)
            {
                if ((activeGatt != null) && (activeGatt == activeGatt))
                {
                    ARSALPrint.d(TAG, "activeGatt disconnected" );
                    
                    activeGatt.close();
                    activeGatt = null;
                    
                    /* Post disconnectionSem only if the disconnect is asked */
                    if (askDisconnection)
                    {
                        disconnectionSem.release();
                    }
                    
                    /* if activePeripheral is discovering services */
                    if (isDiscoveringServices)
                    {
                        discoverServicesError = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_BLE_NOT_CONNECTED;
                        discoverServicesSem.release();
                    }
                    
                    /* if activePeripheral is discovering Characteristics */
                    if (isDiscoveringCharacteristics)
                    {
                        discoverCharacteristicsError = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_BLE_NOT_CONNECTED;
                        discoverCharacteristicsSem.release();
                    }
                    
                    /* if activePeripheral is configuring Characteristics */
                    if (isConfiguringCharacteristics)
                    {
                        configurationCharacteristicError = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_BLE_NOT_CONNECTED;
                        configurationSem.release();
                    }
                    
                    /* Notify listener */
                    if (!askDisconnection)
                    {
                        if (listener != null)
                        {
                            listener.onBLEDisconnect();
                        }
                    }
                }
            }
        }
        
        @Override
        // New services discovered
        public void onServicesDiscovered (BluetoothGatt gatt, int status)
        {
            if (status != BluetoothGatt.GATT_SUCCESS)
            {
                /* the discovery is not successes */
                discoverServicesError = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_BLE_SERVICES_DISCOVERING;
            }
            
            discoverServicesSem.release();
        }
        
        @Override
        /* Result of a characteristic read operation */
        public void onCharacteristicRead (BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status)
        {
            //Do Nothing
        }
        
        @Override
        public void onDescriptorRead (BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status)
        {
            //Do Nothing
        }
        
        @Override
        public void onDescriptorWrite (BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status)
        {
            /* check the status */
            if (status != BluetoothGatt.GATT_SUCCESS)
            {
                configurationCharacteristicError = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_BLE_CHARACTERISTIC_CONFIGURING;
            }
            
            /* post a configuration Semaphore */
            configurationSem.release();
        }
        
        @Override
        /* Characteristic notification */
        public void onCharacteristicChanged (BluetoothGatt gatt, BluetoothGattCharacteristic characteristic)
        {
            readCharacteristicMutex.lock();
            characteristicNotifications.add(characteristic);
            readCharacteristicMutex.unlock();
            
            /* post a readCharacteristic Semaphore */
            readCharacteristicSem.release();
        }
    };
    
    public ARNETWORKAL_ERROR_ENUM setCharacteristicNotification (BluetoothGattService service, BluetoothGattCharacteristic characteristic)
    {
        ARNETWORKAL_ERROR_ENUM result = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK; 
        synchronized (this)
        {
            BluetoothGatt localActiveGatt = activeGatt;
            
            /* If there is an active Gatt, disconnect it */
            if(localActiveGatt != null)
            {
                isConfiguringCharacteristics = true;
                configurationCharacteristicError = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_OK;
                
                boolean notifSet = localActiveGatt.setCharacteristicNotification (characteristic, true);
                BluetoothGattDescriptor descriptor = characteristic.getDescriptor(ARNETWORKALBLEMANAGER_CHARACTERISTIC_UPDATE_NOTIFICATION_DESCRIPTOR_UUID);
                boolean valueSet = descriptor.setValue (BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                boolean descriptorWriten = localActiveGatt.writeDescriptor (descriptor);
                /* wait the configuration semaphore*/
                try
                {
                    configurationSem.acquire ();
                    result = configurationCharacteristicError;
                    
                }
                catch (InterruptedException e)
                {
                    e.printStackTrace();
                    result = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR;
                }
                
                isConfiguringCharacteristics = false;
            }
            else
            {
                result = ARNETWORKAL_ERROR_ENUM.ARNETWORKAL_ERROR_BLE_NOT_CONNECTED;
            }
            
        }
        
        return result;
    }
    
    public boolean writeData (byte data[], BluetoothGattCharacteristic characteristic)
    {
        boolean result = false;
        
        BluetoothGatt localActiveGatt = activeGatt;
        if ((localActiveGatt != null) && (characteristic != null) && (data != null))
        {
            characteristic.setValue(data);
            result = localActiveGatt.writeCharacteristic(characteristic);
        }
        
        return result;
    }
    
    public boolean readData (List<BluetoothGattCharacteristic> characteristicArray)
    {
        boolean result = false;
        
        /* wait the readCharacteristic semaphore*/
        try
        {
            readCharacteristicSem.acquire ();
            
            if  (characteristicNotifications.size() > 0)
            {
                readCharacteristicMutex.lock();
                
                characteristicArray.addAll(characteristicNotifications);
                characteristicNotifications.clear();
                
                readCharacteristicMutex.unlock();
                
                result = true;
            }
        }
        catch (InterruptedException e)
        {
            e.printStackTrace();
        }
        
        return result;
    }
    
    public void unlock ()
    {
        /* post all Semaphore to unlock the all the functions */
        connectionSem.release();
        configurationSem.release();
        readCharacteristicSem.release();
        /* disconnectionSem is not post because:
         * if the connection is fail, disconnect is not call.
         * if the connection is successful, the BLE callback is always called.
         * the disconnect function is called after the join of the network threads.
         */
    }
    
    public void reset ()
    {
        synchronized (this)
        {
            
            /* reset all Semaphores */
            
            while (connectionSem.tryAcquire() == true)
            {
                /* Do nothing*/
            }
            
            while (disconnectionSem.tryAcquire() == true)
            {
                /* Do nothing*/
            }
            
            while (discoverServicesSem.tryAcquire() == true)
            {
                /* Do nothing*/
            }
            
            while (discoverCharacteristicsSem.tryAcquire() == true)
            {
                /* Do nothing*/
            }
            
            while (readCharacteristicSem.tryAcquire() == true)
            {
                /* Do nothing*/
            }
            
            while (configurationSem.tryAcquire() == true)
            {
                /* Do nothing*/
            }
        }
    }
}
