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
import android.util.Log;

import android.bluetooth.BluetoothAdapter;
import android.content.IntentFilter;
import android.content.pm.PackageManager;

@TargetApi(18)
public class ARNetworkALBLEManager
{
    private static String TAG = "ARNetworkALBLEManager";
    
    private static final UUID ARNETWORKALBLEMANAGER_CHARACTERISTIC_UPDATE_NOTIFICATION_DESCRIPTOR_UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");
    
    private ArrayList<ArrayList<BluetoothGattCharacteristic>> mGattCharacteristics;
    
    private Context context;
    private BluetoothDevice deviceBLEService;
    private BluetoothGatt gatt;
    
    boolean gattConnected = false;
    
    private List<BluetoothGattCharacteristic> characteristicNotifications;
    
    private Semaphore connectSem;
    private Semaphore disconnectSem;
    private Semaphore configurationSem;
    private Semaphore readCharacteristicSem;
    
    private Lock readCharacteristicMutex;
    
    private int configurationStatus;
    
    
    /**
     * Constructor
     */
    public ARNetworkALBLEManager (Context context)
    {
        Log.i(TAG, "new ARNetworkALBLEManager ");
        
        this.context = context;
        this.deviceBLEService =  null;
        this.gatt = null;
        
        characteristicNotifications = new ArrayList<BluetoothGattCharacteristic> ();
        
        connectSem = new Semaphore (0);
        disconnectSem = new Semaphore (0);
        configurationSem = new Semaphore (0);
        readCharacteristicSem = new Semaphore (0);
        
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
    public boolean connect (BluetoothDevice deviceBLEService)
    {
        Log.d(TAG, "connect ");
        
        /* if there is an active gatt, disconnecting it */
        if (this.gatt != null) 
        {
            disconnect();
        }
        
        this.deviceBLEService = deviceBLEService;
        this.gatt = deviceBLEService.connectGatt (context, false, gattCallback);
        
        //Log.d(TAG, "wait connect semaphore" );
        /* wait the connect semaphore*/
        try
        {
            connectSem.acquire ();
        }
        catch (InterruptedException e)
        {
            e.printStackTrace();
        }
        
        /* return false if the gatt is null*/
        return (this.gatt != null);
    }
    
    public void disconnect ()
    {
        Log.i(TAG, "disconnect");
        if (gatt != null)
        {
            if (gattConnected == true)
            {
                /* Disconnect the get if the connection is successful */
                Log.i(TAG, "the gatt is Connected ... disconnect it");
                
                gatt.disconnect();
                
                Log.i(TAG, "wait the disconnect Semaphore");
                
                /* wait the disconnect Semaphore*/
                try
                {
                    disconnectSem.acquire ();
                }
                catch (InterruptedException e)
                {
                    e.printStackTrace();
                }
            }
            
            Log.i(TAG, "gatt.close() ... ");
            gatt.close();
            gatt = null;
            
        }
    }
    
    public BluetoothGatt getGatt ()
    {
        return gatt;
    }
    
    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback()
    {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState)
        {
            //Log.d(TAG, "BLE onConnectionStateChange");
            
            String intentAction;
            if (newState == BluetoothProfile.STATE_CONNECTED)
            {
                //Log.d(TAG, "Connected to GATT server.");
                
                /* discover the gatt services */
                discoverBLENetworkServices (gatt);
                
                gattConnected = true;
            }
            else if (newState == BluetoothProfile.STATE_DISCONNECTED)
            {
                Log.d(TAG, "gatt disconnected" );
                
                //Log.d(TAG, "post a disconnect Semaphore" );
                /* post a disconnect Semaphore */
                disconnectSem.release();
                
                gattConnected = false;
            }
        }
        
        @Override
        // New services discovered
        public void onServicesDiscovered (BluetoothGatt gatt, int status)
        {
            //Log.d(TAG, "onServicesDiscovered");
            
            if (status == BluetoothGatt.GATT_SUCCESS)
            {
                //Log.d(TAG, "number of servies: "+ gatt.getServices().size());
            }
            else
            {
                Log.e(TAG, "servicesDiscovered status: " + status);
                /* if the discovery is not successes */
                
                /* disconnect the gatt */
                disconnect ();
            }
            
            //Log.d(TAG, "post a connect Semaphore" );
            /* post a connectSem */
            connectSem.release();
            
        }
        
        @Override
        /* Result of a characteristic read operation */
        public void onCharacteristicRead (BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status)
        {
            //Log.d(TAG, "onCharacteristicRead");
        }
        
        @Override
        public void onDescriptorRead (BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status)
        {
            //Log.d(TAG, "onDescriptorRead");
        }
        
        @Override
        public void onDescriptorWrite (BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status)
        {
            Log.d(TAG, "onDescriptorWrite");
            Log.d(TAG, "status: " + status);
            
            /* write the configurationStatus */
            configurationStatus = status;
            
            Log.d(TAG, "post a configuration Semaphore" );
            /* post a configuration Semaphore */
            configurationSem.release();
        }
        
        @Override
        /* Characteristic notification */
        public void onCharacteristicChanged (BluetoothGatt gatt, BluetoothGattCharacteristic characteristic)
        {
            Log.d(TAG, "BLE onCharacteristicChanged");
            //Log.d(TAG, "characteristic.getUuid(): " + characteristic.getUuid());
            
            readCharacteristicMutex.lock();
            characteristicNotifications.add(characteristic);
            readCharacteristicMutex.unlock();
            
            //Log.d(TAG, "post a readCharacteristic Semaphore" );
            /* post a readCharacteristic Semaphore */
            readCharacteristicSem.release();
            
        }
        
        private void discoverBLENetworkServices (BluetoothGatt gatt)
        {
            //Log.d(TAG, "BLE discoverBLENetworkServices" );
            
            boolean discoveryRes = false;
            
            /* run the discovery of the gatt services */
            discoveryRes = gatt.discoverServices();
            
            if (discoveryRes == false)
            {
                /* the gatt discovering is not running */
                
                /* disconnect the gatt */
                disconnect ();
                
                /* stop the connecting process */
                //Log.d(TAG, "post a connect Semaphore" );
                /* post a connectSem */
                connectSem.release();
            }
        }
        
    };
    
    public boolean setCharacteristicNotification (BluetoothGattService service, BluetoothGattCharacteristic characteristic)
    {
        //Log.d(TAG, "setCharacteristicNotification");
        
        Boolean result = false;
        
        if (gatt != null)
        {
            
            boolean notifSet = gatt.setCharacteristicNotification (characteristic, true);
            
            BluetoothGattDescriptor descriptor = characteristic.getDescriptor(ARNETWORKALBLEMANAGER_CHARACTERISTIC_UPDATE_NOTIFICATION_DESCRIPTOR_UUID);
            
            boolean valueSet = descriptor.setValue (BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
            
            boolean descriptorWriten = gatt.writeDescriptor (descriptor);
            
            Log.d(TAG, "wait configuration semaphore" );
            /* wait the configuration semaphore*/
            try
            {
                configurationSem.acquire ();
                
                /* check the configurationStatus */
                if (configurationStatus == BluetoothGatt.GATT_SUCCESS)
                {
                    result = true;
                }
                
            }
            catch (InterruptedException e)
            {
                e.printStackTrace();
            }
        }
        return result;
    }
    
    public boolean writeData (byte data[], BluetoothGattCharacteristic characteristic)
    {
        //Log.d(TAG, "writeData" );
        
        boolean result = false;
        
        if ((gatt != null) && (characteristic != null) && (data != null))
        {
            //Log.d(TAG, "ok write writeCharacteristic value" );
            characteristic.setValue(data);
            gatt.writeCharacteristic(characteristic);
            result = true;
        }
        
        return result;
    }
    
    public boolean readData (List<BluetoothGattCharacteristic> characteristicArray)
    {
        //Log.d(TAG, "readData" );
        
        boolean result = false;
        
        //Log.d(TAG, "wait readCharacteristic semaphore" );
        /* wait the readCharacteristic semaphore*/
        try
        {
            readCharacteristicSem.acquire ();
            
            if  (characteristicNotifications.size() > 0)
            {
                //Log.d(TAG, "lock readCharacteristicMutex " );
                readCharacteristicMutex.lock();
                
                characteristicArray.addAll(characteristicNotifications);
                characteristicNotifications.clear();
                
                readCharacteristicMutex.unlock();
                //Log.d(TAG, "unlock readCharacteristicMutex " );
                
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
        connectSem.release();
        configurationSem.release();
        readCharacteristicSem.release();
        /* disconnectSem is not post because:
         * if the connection is fail, disconnect is not call.
         * if the connection is successful, the BLE callback is always called.
         * the disconnect function is called after the join of the network threads.
         */
    }
    
}
