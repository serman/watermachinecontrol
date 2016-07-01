from __future__ import print_function
import cv2
import sys
import time 
import requests
import operator
import numpy as np


mresult=None
lastResult=None

def sendData(img):
    print ("sending");
    data= cv2.imencode('.jpg', img)[1].tostring()

    # Draw a rectangle around the faces
    #for (x, y, w, h) in faces:
    #    cv2.rectangle(frame, (x, y), (x+w, y+h), (0, 255, 0), 2)

        # Face detection parameters
    params = { 'returnFaceAttributes': 'age,gender', 
               'returnFaceLandmarks': 'true'} 

    headers = dict()
    headers['Ocp-Apim-Subscription-Key'] = _key
    headers['Content-Type'] = 'application/octet-stream'

    json = None

    result = processRequest( json, data, headers, params )
    

    # Display the resulting frame
    #cv2.imshow('Video', frame)
    if result is not None:
        # Load the original image from disk
        data8uint = np.fromstring( data, np.uint8 ) # Convert string to an unsigned int array
        img = cv2.cvtColor( cv2.imdecode( data8uint, cv2.IMREAD_COLOR ), cv2.COLOR_BGR2RGB )

        renderResultOnImage( result, img )

        ig, ax = plt.subplots(figsize=(15, 20))
        #cv2.imshow('Video', img)  
    
    return result;


def processRequest( json, data, headers, params ):

    """
    Helper function to process the request to Project Oxford

    Parameters:
    json: Used when processing images from its URL. See API Documentation
    data: Used when processing image read from disk. See API Documentation
    headers: Used to pass the key information and the data type request
    """

    retries = 0
    result = None

    while True:
        response = requests.request( 'post', _url, json = json, data = data, headers = headers, params = params )

        if response.status_code == 429: 

            print( "Message: %s" % ( response.json()['error']['message'] ) )

            if retries <= _maxNumRetries: 
                time.sleep(1) 
                retries += 1
                continue
            else: 
                print( 'Error: failed after retrying!' )
                break

        elif response.status_code == 200 or response.status_code == 201:

            if 'content-length' in response.headers and int(response.headers['content-length']) == 0: 
                result = None 
            elif 'content-type' in response.headers and isinstance(response.headers['content-type'], str): 
                if 'application/json' in response.headers['content-type'].lower(): 
                    result = response.json() if response.content else None 
                elif 'image' in response.headers['content-type'].lower(): 
                    result = response.content
        else:
            print( "Error code: %d" % ( response.status_code ) )
            print( "Message: %s" % ( response.json()['error']['message'] ) )

        break
        
    return result

def renderResultOnImage( result, img ):
    
    """Display the obtained results onto the input image"""

    #mresult=result;
    print("Faces in image "  + str(len(result)))
    
    for currFace in result:
        faceRectangle = currFace['faceRectangle']
        cv2.rectangle( img,(faceRectangle['left'],faceRectangle['top']),
                           (faceRectangle['left']+faceRectangle['width'], faceRectangle['top'] + faceRectangle['height']),
                       color = (255,0,0), thickness = 1 )

        faceLandmarks = currFace['faceLandmarks']

        for _, currLandmark in faceLandmarks.items():
            cv2.circle( img, (int(currLandmark['x']),int(currLandmark['y'])), color = (0,255,0), thickness= -1, radius = 1 )

    for currFace in result:
        print(str(currFace['faceAttributes']))
        #print(str(currFace['faceAttributes']))
        #faceRectangle = currFace['faceRectangle']
        #faceAttributes = currFace['faceAttributes']

        #textToWrite = "%c (%d)" % ( 'M' if faceAttributes['gender']=='male' else 'F', faceAttributes['age'] )
        #cv2.putText( img, textToWrite, (faceRectangle['left'],faceRectangle['top']-15), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255,255,255), 1 )



cascPath = sys.argv[1]
faceCascade = cv2.CascadeClassifier(cascPath)

video_capture = cv2.VideoCapture(0)
# Import library to display results
import matplotlib.pyplot as plt
#%matplotlib inline 
_url = 'https://api.projectoxford.ai/face/v1.0/detect'
_key = '726500e04dc94de9a75e33ddd9310cfc'
_maxNumRetries = 10
pause=False;

while True:
    # Capture frame-by-frame
    #if(pause==True):
    #    time.sleep(5);
    #    pause=False; 

    ret, frame = video_capture.read()

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    faces = faceCascade.detectMultiScale(
        gray,
        scaleFactor=1.1,
        minNeighbors=5, 
        minSize=(30, 30)
    )
    # Draw a rectangle around the faces
    for (x, y, w, h) in faces:
        cv2.rectangle(frame, (x, y), (x+w, y+h), (0, 255, 0), 2)

    # Display the resulting frame
    
    k= cv2.waitKey(1) & 0xFF
    
    if len(faces) >0:
        #print ("face detectd")        
        if(k == ord('r')):   
            mresult=sendData(frame);        
            if mresult is not None:
                #print("mmmmmmmm not none")   
                lastResult=mresult; 
                mresult=None;
        
    #print(str(lastResult))
    if lastResult is not None:   
        #print("lastresult not none")         
        for currFace in lastResult:            
            faceRectangle = currFace['faceRectangle']
            faceAttributes = currFace['faceAttributes']

            textToWrite = "%c (%d)" % ( 'M' if faceAttributes['gender']=='male' else 'F', faceAttributes['age'] )
            cv2.putText( frame, textToWrite, (faceRectangle['left'],faceRectangle['top']-15), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255,255,255), 1 )

    
    cv2.imshow('Video', frame)   


    



# When everything is done, release the capture
video_capture.release()
cv2.destroyAllWindows()        

