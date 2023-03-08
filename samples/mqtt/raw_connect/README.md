# Raw Connect

[**Return to main sample list**](../../README.md)

This sample is exactly like [Basic Connect](../basic_connect/README.md) in terms of functionality, but it sets up the connection manually in a "raw" way rather than using helper classes that make creating the connection easier. On startup, the device connects to the server using the certificate and key files, and then disconnects. This sample is for reference on connecting via certificate and key files.

Your IoT Core Thing's [Policy](https://docs.aws.amazon.com/iot/latest/developerguide/iot-policies.html) must provide privileges for this sample to connect. Below is a sample policy that can be used on your IoT Core Thing that will allow this sample to run as intended.

<details>
<summary>(see sample policy)</summary>
<pre>
{
  "Version": "2012-10-17",
  "Statement": [
    {
      "Effect": "Allow",
      "Action": [
        "iot:Connect"
      ],
      "Resource": [
        "arn:aws:iot:<b>region</b>:<b>account</b>:client/test-*"
      ]
    }
  ]
}
</pre>

Replace with the following with the data from your AWS account:
* `<region>`: The AWS IoT Core region where you created your AWS IoT Core thing you wish to use with this sample. For example `us-east-1`.
* `<account>`: Your AWS IoT Core account ID. This is the set of numbers in the top right next to your AWS account name when using the AWS IoT Core website.

Note that in a real application, you may want to avoid the use of wildcards in your ClientID or use them selectively. Please follow best practices when working with AWS on production applications using the SDK. Also, for the purposes of this sample, please make sure your policy allows a client ID of `test-*` to connect or use `--client_id <client ID here>` to send the client ID your policy supports.

</details>

## How to run

To run the raw connect sample use the following command:

``` sh
./raw-connect --endpoint <endpoint> --cert <path to the certificate> --key <path to the private key> --user_name <user name to send on connect> --password <password to send on connect>
```

You can also pass a Certificate Authority file (CA) if your certificate and key combination requires it:

``` sh
./raw-connect --endpoint <endpoint> --cert <path to the certificate> --key <path to the private key> --user_name <user name to send on connect> --password <password to send on connect> --ca_file <path to root CA>
```