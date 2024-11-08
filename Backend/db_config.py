from pymongo import MongoClient
import os

MONGO_URI = os.getenv('MONGO_URI', 'mongodb+srv://arnavanand710:1234@cluster0.nlt9q.mongodb.net/?retryWrites=true&w=majority&appName=Cluster0')
DB_NAME = 'IOT_CP' 

client = MongoClient(MONGO_URI)
db = client[DB_NAME]

def get_db():
    return db 
