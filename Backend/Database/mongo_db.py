from pymongo import MongoClient

class MongoDB:
    def __init__(self, uri, db_name):
        self.client = MongoClient(uri)
        self.db = self.client[db_name]
        self.collection = self.db['user_retention_data']  # Collection name

    def insert_user_retention(self, rssi_values, user_retention):
        document = {
            'rssi_values': rssi_values,
            'user_retention': user_retention
        }
        result = self.collection.insert_one(document)
        return str(result.inserted_id)

    def get_all_data(self):
        return list(self.collection.find({}))
