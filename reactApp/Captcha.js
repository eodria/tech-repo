import React from 'react';
import ReactDOM from 'react-dom';
import $ from 'jquery';
import './stack.css';
import Stack from './Stack.js';

class Captcha extends React.Component {
  constructor(props) {
    super(props);
    this.state = { 
    }
  }
  render() {
    return (
      <div class="info">
        
        <h1>Google reCAPTCHA</h1>
        <p>reCAPTCHA is an API developed by Google.</p>
        <p>It is designed to prevent websites from spam and abuse.</p>
        <p>I have integrated reCAPTCHA into my website for two purposes.</p>
        <p>1. To censor my email from robots to prevent potential abuse</p>
        <p>2. To prevent abuse of automated email on Contact page from evil people.</p>
        <a href="https://www.google.com/recaptcha/intro/v3.html">Click here for further documentation</a>

      </div>

    )
  }
}

export default Captcha;