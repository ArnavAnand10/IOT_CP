from flask import Flask, request, jsonify
from flask_socketio import SocketIO, emit
from flask_cors import CORS  # Import CORS
from pymongo.errors import PyMongoError
from db_config import get_db  # Import get_db from db_config
from datetime import datetime
import pytz  # Import pytz for timezone handling

app = Flask(__name__)

# Enable CORS for all routes
CORS(app)  

# Allow all origins for SocketIO
socketio = SocketIO(app, cors_allowed_origins="*")  

@app.route('/api/data', methods=['GET'])
def get_data():
    try:
        collection = get_db()['user_retention_data']
        data = list(collection.find({}))  # Fetch all documents from the collection
        for document in data:
            document['_id'] = str(document['_id'])  # Convert ObjectId to string for JSON serialization
        return jsonify(data), 200
    except PyMongoError as e:
        return jsonify({"error": "Failed to fetch data from database", "details": str(e)}), 500

@app.route('/api/data', methods=['POST'])
def receive_data():
    if request.is_json:
        data = request.get_json()
        
        # Extract RSSI values and user retention from the received JSON data
        rssi_values = data.get("rssi_values")
        user_retention = data.get("last_retention")

        # Get the current UTC time and convert it to IST
        utc_now = datetime.utcnow()
        ist_timezone = pytz.timezone('Asia/Kolkata')
        ist_now = utc_now.replace(tzinfo=pytz.utc).astimezone(ist_timezone)  # Convert UTC to IST

        # Document structure for MongoDB insertion with IST timestamp
        document = {
            'rssi_values': rssi_values,
            'user_retention': user_retention,
            'timestamp': ist_now.isoformat()  # Save the current time in ISO format
        }

        try:
            # Use get_db to access MongoDB and insert the document
            collection = get_db()['user_retention_data']
            result = collection.insert_one(document)
            print("Data successfully inserted with ID:", result.inserted_id)

            # Emit an event to notify the frontend
            socketio.emit('data_updated')  # Notify the frontend that data has been updated

            return jsonify({"response": "Data received and stored successfully", "inserted_id": str(result.inserted_id)}), 200

        except PyMongoError as e:
            # Handle MongoDB errors and return a JSON response
            print(f"Error during MongoDB insertion: {e}")
            return jsonify({"error": "Failed to insert data into database", "details": str(e)}), 500
    
    else:
        # Return error if the request is not JSON
        return jsonify({"error": "Request must be JSON"}), 400

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000)
