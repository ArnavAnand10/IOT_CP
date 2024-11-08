from flask import Blueprint, request, jsonify
from marshmallow import ValidationError
from Schemas.user_retention_schema import UserRetentionSchema
from Database.mongo_db import MongoDB

# Initialize your database connection (replace with your actual URI)
db = MongoDB('mongodb://localhost:27017/', 'your_database_name')

api = Blueprint('api', __name__)

def insert_user_retention(rssi_values, user_retention):
    return db.insert_user_retention(rssi_values, user_retention)

def get_user_retention_data():
    return db.get_all_data()

@api.route('/user_retention', methods=['POST'])
def add_user_retention():
    schema = UserRetentionSchema()
    try:
        data = schema.load(request.json)
        rssi_values = data['rssi_values']
        user_retention = data['user_retention']

        document_id = insert_user_retention(rssi_values, user_retention)

        return jsonify({"message": "Data added successfully", "id": document_id}), 201

    except ValidationError as err:
        return jsonify(err.messages), 400

@api.route('/user_retention', methods=['GET'])
def get_user_retention():
    data = get_user_retention_data()
    return jsonify(data), 200
