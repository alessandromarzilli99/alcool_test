import json
import boto3

client = boto3.client('iot-data', region_name='us-east-1')

def lambda_handler(event, context):
    value = event['message']
    response = client.publish(
        topic='topic_in',
        qos=1,
        payload=json.dumps({"message":value})
    )
    
    # TODO implement
    return {
        'statusCode': 200,
        'body': json.dumps('Hello from Lambda!')
    }
