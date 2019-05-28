import React from 'react';
import ReactDOM from 'react-dom';
import './resume.css';
import Navbar from './Navbar.js';
import $ from 'jquery';

class Resume extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      hiddenEmail: "od*****@gmail.com",
      unhidden: "odriaes@gmail.com.",
      siteKey: "6LePrpoUAAAAAKGbXdwoAEhP3bLuVd21MegTJQ4G",
      captchaHandler: this.captchaHandler.bind(this)
    };
  }
  captchaHandler() {
    var captchaContainer = null;
    var loadCaptcha = function() {
      captchaContainer = grecaptcha.render('captcha_container', {
        'sitekey': "6LePrpoUAAAAAKGbXdwoAEhP3bLuVd21MegTJQ4G",
        'callback': function(response) {
          console.log(response);
          $('#email').html('odriaes@gmail.com');
          fetch('http://localhost:8081/submit', {
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
              this.setState({hiddenEmail: 'odriaes@gmail.com'});
              $('#captcha_container').html(); // or .html('') ???
            }
          });
        }
      });
    };
    loadCaptcha();
  }
	render() {
        return (
            <div>

                <Navbar />

                <div class="name-email">
                  <h2 class="header">Estevon Odria</h2>
                  <p id="email">Contact email: {this.state.hiddenEmail}</p>
                  <div id="captcha_container"></div>
                  <p class="captcha" onClick={this.state.captchaHandler}><i>(click here to uncensor email)</i></p>
                </div>

                <div name class="wrapper">
                  <span class="resumeHeader">
                    <h2 class="header">EDUCATION</h2>
                  </span>

                  <span class="body">
                    <p>Carnegie Mellon University Pittsburgh, PA</p>
                    <i>Bachelor of Science, Electrical and Computer Engineering</i>
                    <i> May 2018</i>
                  </span>
                </div>


                <div class="work-wrapper">

                  <span class="work-resumeHeader">
                    <h2 class="work-header">WORK EXPERIENCE</h2>
                  </span>

                  <span class="work-body">
                    <p>Esurance San Francisco, CA</p>
                    <i>Software Engineer</i>
                    <i> November 2018 – Present</i>
                  </span>
                
                  <ul class="resume-list">
                    <li>Implemented a Digital Wallet to centralize and manage all customer payment information across Esurance applications</li>
                    <li>Set up API endpoints to support Zelle and card payment methods.</li>
                    <li>Set up online Mongo database to maintain payment methods and customer information</li>
                    <li>Integrating Wallet API with external Mastercard Send API's to tokenize sensitive customer information</li>
                  </ul>
                  
                  <span name="esurance-intern" class="work-body">
                    <p>Esurance San Francisco, CA</p>
                    <i>Software Engineering Intern</i>
                    <i> May 2018 – August 2018</i>
                  </span>

                  <ul class="resume-list">
                    <li>Implemented a payment service software system</li>
                    <li>Implemented client ReactJS payment form and a back-end API service</li>
                    <li>Invokes external third-party encryption algorithms to hide card information</li>
                    <li>Integrated Apache Kafka messaging system for efficient payment status notifications</li>
                    <li>Introduced object-oriented methodologies, encapsulation principles and design patterns to IT Development term via summer presentations.</li>
                  </ul>

                  <span name="uhg-intern" class="work-body">
                    <p>UnitedHealth Group Greater Chicago Area</p>
                    <i>Technology Development Intern</i>
                    <i> May 2017 – August 2017</i>
                  </span>

                  <ul class="resume-list">
                    <li>Designed and implemented web application to replace Outlook booking system</li>
                    <li>Created interface to let clients view entire floor map instead of previous five room limit</li>
                    <li>Implemented AJAX for asynchronous and real-time filtering without page reloading</li>
                    <li>Implemented back-end logic to toggle room colors (green/red> based on availability</li>
                    <li>Managed SQL database for integration with rooms for seamless filtering</li>
                    <li>Proposed business case to investors to highlight massive cost-reduction benefits of application</li>
                  </ul>

                </div>

                <div class="tech-wrapper">

                  <span class="work-resumeHeader">
                    <h2 class="work-header">TECHNOLOGIES</h2>
                  </span>

                  <ul class="tech">
                    <li>Java</li>
                    <li>Python</li>
                    <li>Javascript</li>
                    <li>ReactJS</li>
                    <li>NodeJS</li>
                    <li>C</li>
                    <li>C#</li>
                  </ul>

                  <ul class="tech">
                    <li>Windows</li>
                    <li>MacOS</li>
                    <li>Linux</li>
                    <li>Unix</li>
                  </ul>

                  <ul class="tech">
                    <li>MongoDB</li>
                    <li>MySQL</li>
                    <li>SQL</li>
                  </ul>

              </div>

              <div class="footer">
                <p class="footer">© Estevon Odria 2019 | All rights reserved</p>
                <i class="fa fa-linkedin-square"></i>
                <i class="fa fa-github-square"></i>
              </div>

            </div>
        );
	}
}

export default Resume;