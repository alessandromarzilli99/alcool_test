import json
import boto3
import time

client=boto3.client('dynamodb')

def lambda_handler(event, context):
    current_time = int(time.time())
    data = client.scan(
        TableName='alcooltest',
        FilterExpression='ts >= :value',
        ExpressionAttributeValues={
            ':value': {
                'N': str(current_time-604800)+"000"
            }
        }
    )
    
    return {
        'statusCode': 200,
        'body': json.dumps(data["Items"])
    }