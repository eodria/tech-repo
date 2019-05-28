import React from 'react';
import reactDOM from 'react-dom';
import Navbar from './Navbar.js'
import './siteStyle.css';
import $ from 'jquery';

class Contact extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      fullname: 'John Doe',
      email: 'Enter email',
      phone: 'xxx-xxx-xxxx format',
      text: 'Enter text here',
      captchaSuccess: false,
      isChecked: false,
      dateStamp: ''
    }
    this.submitButtonHandler = this.submitButtonHandler.bind(this);
    this.handlePhoneChange = this.handlePhoneChange.bind(this);
    this.handleEmailChange = this.handleEmailChange.bind(this);
    this.handleNameChange = this.handleNameChange.bind(this);
    this.handleTextChange = this.handleTextChange.bind(this);
    this.checkboxHandler = this.checkboxHandler.bind(this);
  }
  checkboxHandler() {
    this.setState({isChecked: !this.state.isChecked});
  }
  submitButtonHandler(e) {
    e.preventDefault();
    var stamp = new Date().toString();
    this.setState({ dateStamp: stamp }, () => {
    var emailRegex = /^(([^<>()\[\]\\.,;:\s@"]+(\.[^<>()\[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
    if (emailRegex.test(this.state.email)) {
      console.log('email match!');
      var url = 'http://52.52.115.98:8081/sendEmail';
      console.log('url: ' + url);
      fetch(url, {
        body: JSON.stringify(this.state),
        cache: 'no-cache',
        credentials: 'same-origin',
        headers: {
          'content-type': 'application/json',
          'Access-Control-Allow-Origin': '*'
        },
        method: 'POST',
        mode: 'cors',
        redirect: 'follow',
        referrer: 'no-referrer'
      })
      .then(function(response) {
        console.log(response);
      });
    }
    else {
      console.log('invalid email');
    }
    var phoneRegex = /^(\()?\d{3}(\))?(-|\s)?\d{3}(-|\s)\d{4}$/;
    if (phoneRegex.test(this.state.phone)) {
      console.log('phone match!');
      fetch('http://52.52.115.98:8081/sendSMS', {
        body: JSON.stringify(this.state),
        cache: 'no-cache',
        credentials: 'same-origin',
        headers: {
          'content-type': 'application/json',
        },
        method: 'POST',
        mode: 'cors',
        redirect: 'follow',
        referrer: 'no-referrer'
      })
      .then(function(response) {
        console.log(response);
        //return response.json();
      })
      //.then(function(myJson) {
      //  console.log(myJson);
      //})
    }
    else {
      console.log('invalid phone number!');
    }
  });
    }
    
  handleEmailChange(e) {
    this.setState({email: event.target.value});
  }
  handlePhoneChange(e) {
    this.setState({phone: event.target.value});
  }
  handleNameChange(e) {
    this.setState({fullname: event.target.value});
  }
  handleTextChange(e) {
    this.setState({text: event.target.value});
  }
  captchaHandler() {
    fetch('http://52.52.115.98:8081/submit', {
      body: JSON.stringify(this.state),
      cache: 'no-cache',
      credentials: 'same-origin',
      headers: {
        'content-type': 'application/json',
      },
      method: 'POST',
      mode: 'cors',
      redirect: 'follow',
      referrer: 'no-referrer'
    })
    .then(function(response) {
      console.log(response);
      return response.json();
    })
    .then(function(myJson) {
      console.log(myJson);
      if (myJson.responseCode == '0') {
        this.setState({ captchaSuccess: true });
      }
    });
  }
	render() {
		return (
      <div>

        <Navbar />

            <form class="input">
    
              <div class="form-group contact">
                <label for="fullname">Full name</label>
                <input onChange={this.handleNameChange} type="fullname" class="form-control" id="fullname" aria-describedby="nameHelp" placeholder={this.state.fullname}>
                </input>
              </div>
              <div class="form-group contact">
                <label for="exampleInputEmail1">Email address</label>
                <input onChange={this.handleEmailChange} type="email" class="form-control" id="exampleInputEmail1" aria-describedby="emailHelp" placeholder={this.state.email}>
                </input>
                <small id="emailHelp" class="form-text text-muted">My site will send you an automated email.</small>
              </div>
              <div class="form-group contact">
                <label for="Phone number">Phone number</label>
                <input onChange={this.handlePhoneChange} type="phone" class="form-control" id="phone" aria-describedby="emailHelp" placeholder={this.state.phone}>
                </input>
                <small id="phoneHelp" class="form-text text-muted">My site can also send you a quick SMS text.</small>
              </div>
              <div class="form-group contact" name="textarea">
                <label name="textarea" for="exampleFormControlTextarea1">Large textarea</label>
                <textarea onChange={this.handleTextChange} class="form-control rounded-0" id="exampleFormControlTextarea1" rows="10"
                 cols="50"
                 placeholder={this.state.text}></textarea>
              </div>

              <div class="captcha-wrapper">

                <div class="form-check">
                  <input type="checkbox" checked={this.state.isChecked} onChange={() => this.checkboxHandler()} class="form-check-input" id="resumeCheck"/>
                  <label class="form-check-label" for="resumeCheck">Check if you would like my resume attached to email</label>
                </div>

                <div class="form-group contact" id="button">
                  <button onClick={this.submitButtonHandler} name="submit" 
                   type="submit" class="btn btn-primary">Submit</button>
                </div>

                <div id="contact-captcha" class="g-recaptcha" data-sitekey="6LePrpoUAAAAAKGbXdwoAEhP3bLuVd21MegTJQ4G"></div>

              </div>

            </form>

        <div class="footer">
          <p class="footer">© Estevon Odria 2019 | All rights reserved</p>
          <a class="footer" href="https://www.linkedin.com/in/estevon-o-315483132/"><i class="fa fa-linkedin-square"></i></a>
          <a class="footer" href="https://github.com/eodria/tech-repo"><i class="fa fa-github-square"></i></a>
        </div>

		  </div>
		
		);
	}
}
export default Contact;