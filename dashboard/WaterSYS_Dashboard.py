import paho.mqtt.client as mqtt			# Import paho MQTT library
import time					# The time library is useful for delays
import tkinter as tk #tkinter import
 

root = tk.Tk() #tkinter object
root.title('Automatic Watering System') #Window title
root.geometry('{}x{}'.format(800, 600)) #Window size

    
    
# "on message" event
def on_message (client, userdata, msg):
	topic = str(msg.topic)
	payload = str(msg.payload.decode('UTF-8'))
	
	#allocate data from each feed to topic variables
	if topic == "Temp":
		topicTemp.config(text=payload)
	if topic == "Soil":
		topicSoil.config(text=payload)
	if topic == "UVIdx":
		topicUV.config(text=payload)
	if topic == "Flow":
		topicFlow.config(text=payload)

		
# Create a MQTT client object & connect to broker
client = mqtt.Client("DDD")	
client.on_message = on_message
client.connect("test.mosquitto.org", 1883)	

#subscribe to each published feed
client.subscribe("Temp")			
client.subscribe("Soil")	
client.subscribe("UVIdx")		
client.subscribe("Flow")		
	
#populate window grid with data labels and values
tk.Label(root, text="Ambient Temp (degC)", font=('fixed', 20)).grid(row=1,column=1)
topicTemp = tk.Label(root, font=('fixed', 20),)
topicTemp.grid(sticky=tk.N, row=1, column=2, padx=5, pady=(20,20))

tk.Label(root, text="Soil Moisture", font=('fixed', 20)).grid(row=2,column=1)
topicSoil = tk.Label(root, font=('fixed', 20),)
topicSoil.grid(sticky=tk.N, row=2, column=2, padx=5, pady=(20,20))

tk.Label(root, text="UV Index", font=('fixed', 20)).grid(row=3,column=1)
topicUV = tk.Label(root, font=('fixed', 20),)
topicUV.grid(sticky=tk.N, row=3, column=2, padx=5, pady=(20,20))

tk.Label(root, text="Flow Rate (L/Hr)", font=('fixed', 20)).grid(row=4,column=1)
topicFlow = tk.Label(root, font=('fixed', 20),)
topicFlow.grid(sticky=tk.N, row=4, column=2, padx=5, pady=(20,20))


# Start the MQTT client & loop
client.loop_start()				
root.mainloop()

# Main program loop
while(1):
	time.sleep(1)				# Sleep for a second
	
