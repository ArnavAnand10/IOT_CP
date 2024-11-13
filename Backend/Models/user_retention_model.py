from Database.mongo_db import MongoDB

# Initialize your database connection (replace with your actual URI)
db = MongoDB('mongodb://localhost:27017/', 'your_database_name')

def insert_user_retention(rssi_values, user_retention):
    return db.insert_user_retention(rssi_values, user_retention)

def get_user_retention_data():
    return db.get_all_data()


