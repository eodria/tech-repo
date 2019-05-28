import React from 'react';
import ReactDOM from 'react-dom';
import $ from 'jquery';
import './stack.css';

class Twilio extends React.Component {
  constructor(props) {
    super(props);
    this.state = { }
  }
  render() {
    return (
      <div>
        <h1>Twilio</h1>
        <a href="https://www.twilio.com/">Click here for further documentation</a>
      </div>
    )
  }
}

export default Twilio;