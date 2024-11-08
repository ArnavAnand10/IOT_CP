from marshmallow import Schema, fields

class UserRetentionSchema(Schema):
    rssi_values = fields.List(fields.Integer(), required=True)
    user_retention = fields.Integer(required=True)
    timestamp = fields.DateTime(required=True)  # Add timestamp field
