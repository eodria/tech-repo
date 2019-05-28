const express = require('express');
const path = require('path');
const app = express();

app.use(express.static(path.join(__dirname, 'bundle')));

app.get('/', function(req, res) {
  res.sendFile(path.join(__dirname, 'bundle', 'index.html'));
});

app.listen(8080, function() {
  console.log('App listening at port 8080...');
});