const accountSid = 'ACea56de8d3069f8c63af6e1fbe9e2fb53';
const authToken = 'bb8e7218ebe221684b01f790aa3b03ad';
const client = require('twilio')(accountSid, authToken);
const secretKey = '6LePrpoUAAAAAO2haJYuNmZRVCUIL7tfZjP27UDR';
const resume = [ { filename: 'resume.docx', path: 'resume_curr (1).docx' } ];

var counter = 0;

// used to retrieve titles in O(1) time
var dictionary = {
  'App.js': 'Home Page',
  'Stack.js': 'About',
  'Resume.js': 'Resume',
  'Contact.js': 'Contact Me'
};

// load aws sdk
var aws = require('aws-sdk');

// load aws config
aws.config.loadFromPath('config.json');

// load AWS SES
var ses = new aws.SES({ apiVersion: '2010-12-01'});

// send to list
var to = ['eodria@andrew.cmu.edu'];

// this must relate to a verified SES account
var from = 'odriaes@gmail.com';

var http = require('http');
var express = require('express');
var bodyParser = require('body-parser');
var app = express();
var fs = require('fs');

const cors = require('cors');

app.use(bodyParser.json());
app.use(cors());

/* parameters for connecting to mongo collections */
var MongoClient = require('mongodb').MongoClient;
var url = 'mongodb://localhost:27017/';
var dbo = null;

var server = app.listen(8081, function() {
    var host = server.address().address;
    var port = server.address().port;
    console.log('bitch work!');
    console.log("Example app listening at http://%s:%s", host, port)
});

app.post('/oauth', function(req, res) {
  console.log('received oath...');
  console.log(req.body);
});

app.get('/oauth', function(req, res) {
  console.log('received oath...');
  console.log(req.body);
});

app.get('/', function(req, res) {
  res.send('hey hey hey');
});

// testing url
app.get('/test', function(req, res) {
    res.send('testing...');
});

function parseRestOfSentence(accumulator, fileText, i) {
  while (fileText.charAt(i) != '.') {
    accumulator += fileText.charAt(i);
    i++;
  }
  accumulator += '.';
  return accumulator;
}

// note to self: changed from findOccurrences to search
// also commented out some stuff before
//function findOccurrences(fileName, fileText, searchToken) {
function search(document, searchToken) {
  //fileText = fileText.split("return (")[1];
  //cleanText = fileText.replace(/<\/?[^>]+(>|$)/g, ""); //.replace(/\s/g, "");
  //console.log('outputting mongo document...');
  console.log(document);
  var result = [];
  var cleanText = document['text'];
  console.log("cleanText: " + cleanText);
  var hi = cleanText.length - searchToken.length;
  var currentSentence = '';
  // must loop through to get the sentence it is within
  var i = 0;
  var result = [];
  var currentSentence = '';
  
  while (i < hi) {
    var substring = cleanText.substring(i, i + searchToken.length);
    // try this
    var char = cleanText.charAt(i);
    
    // retrieve title from map
    // make key the sentence
    if (substring == searchToken) {
      //var title = dictionary[fileName];
      var title = document['title'];
      var link = document['link']
      //currentSentence += '<b>' + substring + '</b>';
      //var sentence = parseRestOfSentence(currentSentence, cleanText, i+1);
      currentSentence += substring;
      var sentence = parseRestOfSentence(currentSentence, cleanText, i+searchToken.length);
      result[result.length] = 
        { "title" : title, "sentence" : sentence, "link" : link };
      currentSentence = '';
    }
    else if (char == '.') {
      currentSentence = '';
    }
    else {
      currentSentence += cleanText.charAt(i); 
    }
    i++;
  }
  return result;
}

function findOcurrences(searchToken, res) {
  var matches = [ ];
  MongoClient.connect(url)
    .then(function(db) {
      //if (err) throw err;
      var dbo = db.db('cache');
      dbo.collection('pages').find({}).toArray(function(err, results) {
        var lol = null;
        if (err) throw err;
        console.log(results);
        // we must go through each file and append to matches
        for (var i = 0; i < results.length; i++) {
          var hits = search(results[i], searchToken);
          console.log('hits: ' + JSON.stringify(hits));
          console.log('database is failing haha');
          for (var j = 0; j < hits.length; j++) {
            matches[matches.length] = hits[j];
          }
        }
        db.close();
        //lol = matches;
        //return lol;
        // return it here... ???
        res.json(matches);
      })
      /*.then(() => {
        console.log('fuck');
        return result;
      });*/
    });
}

// for now just stick with App.js
function searchForToken(searchToken, res) {
  result = findOcurrences(searchToken, res);
  console.log('race condition: ???');
  //return result;
}

app.post('/search', function(req, res) {
  console.log(req.body);
  var searchToken = req.body.searchToken;
  var matches = searchForToken(searchToken, res);
  //res.json(matches);
});

const data = fs.readFileSync("resume.docx");
const SOURCE_EMAIL = "odriaes@gmail.com";

app.post('/sendEmail', function(req, res) {
  console.log('received request...');
  console.log(req.body);
  var fullname = req.body.fullname;
  var email = req.body.email;
  var msg = req.body.text;
  var isChecked = req.body.isChecked;
  var dateStamp = req.body.dateStamp;
  var subject = 'Automatic reply: Thank you for reaching out!';
  var text = 'Hello, ' + fullname + '!<br/><br/>On ' + dateStamp + ' you sent:<br/><br/>' + msg + '<br/><br/>' +
             'Please allow 2-3 business days for a response. You will find a copy of my resume attached if you chose the option on my website.'
             + '<br/><br/><br/>Warm regards,<br/><br/>Estevon<br>Website Adminstrator<br/>wwww.estevonodria.com';

  var ses_mail = "From: 'estevonodria.com' <" + SOURCE_EMAIL + ">\n";
  ses_mail += "To: " + email + "\n";
  ses_mail += "Subject: Automatic reply: Thank you for reaching out!\n";
  ses_mail += "MIME-Version: 1.0\n";
  ses_mail += "Content-Type: multipart/mixed; boundary=\"NextPart\"\n\n";
  ses_mail += "--NextPart\n";
  ses_mail += "Content-Type: text/html\n\n";
  ses_mail += text + '\n\n';
  ses_mail += "--NextPart\n";
  if (isChecked) {
    ses_mail += "Content-Type: application/octet-stream; name=\"resume.docx\"\n";
    ses_mail += "Content-Transfer-Encoding: base64\n";
    ses_mail += "Content-Disposition: attachment\n\n";
    ses_mail += data.toString("base64").replace(/([^\0]{76})/g, "$1\n") + "\n\n";
    ses_mail += "--NextPart--";
  }
  
  var params = {
    RawMessage: {Data: ses_mail},
    Source: "'Production Amazon SES' <" + SOURCE_EMAIL + ">'"
  };

  ses.sendRawEmail(params, function(err, data) {
    if (err) {
      console.log(err.message);
    } 
    else {
      console.log('Email sent!');
    }
  });

});

app.post('/submit',function(req,res){
  // g-recaptcha-response is the key that browser will generate upon form submit.
  // if its blank or null means user has not selected the captcha, so return the error.
  if(req.body['g-recaptcha-response'] === undefined || req.body['g-recaptcha-response'] === '' || req.body['g-recaptcha-response'] === null) {
    return res.json({"responseCode" : 1,"responseDesc" : "Please select captcha"});
  }
  // req.connection.remoteAddress will provide IP address of connected user.
  var verificationUrl = "https://www.google.com/recaptcha/api/siteverify?secret=" + secretKey + "&response=" + req.body['g-recaptcha-response'] + "&remoteip=" + req.connection.remoteAddress;
  // Hitting GET request to the URL, Google will respond with success or error scenario.
  request(verificationUrl, function(error, response, body) {
    body = JSON.parse(body);
    // Success will be true or false depending upon captcha validation.
    if (body.success !== undefined && !body.success) {
      return res.json({"responseCode" : 1,"responseDesc" : "Failed captcha verification"});
    }
    res.json({"responseCode" : 0,"responseDesc" : "Sucess"});
  });
});

// This will handle 404 requests.
app.use("*",function(req,res) {
  res.status(404).send("404");
});

