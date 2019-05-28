//const Entities = require('html-entities').AllHtmlEntities;
//const entities = new Entities();

var CryptoJS = require('crypto-js');

// must build up the request here...
var consumerKey = 'E46Nvg4-XM0oWXeT5QMtzEyUAIQuEI88EsLGXY_61e14c105!08f625fc16694f74a094e905dfbd06f60000000000000000';
var encodedKey = 'E46Nvg4-XM0oWXeT5QMtzEyUAIQuEI88EsLGXY_61e14c105%2108f625fc16694f74a094e905dfbd06f60000000000000000';
var nonce = '1111111111111111117';
var timestamp = Math.round(new Date().getTime() / 1000);
console.log('timestamp: ' + timestamp);
var method = 'POST';
var requestUrl = 'https%3A%2F%2Fsandbox.api.mastercard.com%2Fsend%2Fv1%2Fpartners%2Fptnr_H8RWWeKXU0nWun6nKWZcLAotVCt%2Fconsumers';
var requestBody = 
 '<consumer><consumer_reference>AB123456</consumer_reference><first_name></first_name><middle_name>Tyler</middle_name><last_name>Smith</last_name><nationality>USA</nationality><date_of_birth>1999-12-30</date_of_birth><address><line1>1 Main St</line1><line2>Apartment 9</line2><city>OFallon</city><country_subdivision>MO</country_subdivision><postal_code>63368</postal_code><country>USA</country></address><primary_phone>11234567890</primary_phone><primary_email>Jane.Smith123@abcmail.com</primary_email></consumer>';
var hash = CryptoJS.SHA256(requestBody).toString(CryptoJS.enc.Base64);
var encodedHash = encodeURIComponent(hash);
console.log('hash: ' + hash + '\n');
console.log('encodedHash: ' + encodedHash + '\n');

// base signature must be properly encoded
var baseString = 
  method + '&amp;' + requestUrl + '&amp;oauth_body_hash%3D' + encodedHash
  + '%26oauth_consumer_key%3D' + encodedKey + '%26oauth_nonce%3D' + nonce 
  + '%26oauth_signature_method%3DRSA-SHA256%26oauth_timestamp%3D' + timestamp + '%26oauth_version%3D1.0';
//var encoded = encodeURIComponent(baseString);
//var encoded = entities.encode(baseString);
var encoded = baseString;
console.log(encoded);

// RSA WITH SHA-256... do this in java later
var signature = 'fCIv+1gvJCCBTkpskHVvolUmQYHZYausy2BrhIG18vklVdZ2Kuh/VM/Gg1QMQ4tCoaCgWt4hpr+mxnz5jelHI9ywMuRypL8pHZPGomjL2K4sPiwFNxXE85j0/ILVMcGFiijH4sD07LuRrZ1uY1slM02EVjfWhGJNCHCXur0WKm0=';
var encodedSignature = encodeURIComponent(signature);
console.log('\n' + encodedSignature + '\n');

// hardcore for now
var header = 
 'oauth_body_hash="' + hash + '",oauth_nonce="' + nonce + '",oauth_signature="' + encodedSignature 
 + '",oauth_consumer_key="' + consumerKey  
 + '",oauth_signature_method="RSA-SHA256",oauth_timestamp="' + timestamp + '",oauth_version="1.0"';

console.log(header);
