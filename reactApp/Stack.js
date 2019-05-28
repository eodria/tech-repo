import React from 'react';
import ReactDOM from 'react-dom';
import Navbar from './Navbar.js';
import $ from 'jquery';
import './stack.css';

// form components
import Captcha from './Captcha.js'; 
import Cloud from './Cloud.js'; 
import ReactInfo from './ReactInfo.js'; 
import Storage from './Storage.js';
import Map from './Map.js'; 
import NodeInfo from './NodeInfo.js'; 
import BackEndServer from './BackEndServer.js';  
import FrontEndServer from './FrontEndServer.js';
import Mongo from './Mongo.js';

class Stack extends React.Component {
  constructor(props) {
  	super(props);
  	this.state = {
  	  form: null 
  	};
  	this.linkHandler = this.linkHandler.bind(this);
  }
  linkHandler(value) {
    this.setState({ form: value });
  }
  render() {
  	return (

  	  <div>

  	    <table>
          <tr>
            <th>Software System Architecture Diagram</th>
          </tr>
  	      <tr>
  	        <th>Front end</th>
  	      </tr>
  	      <tr>
  	        <td>HTML</td>
  	      </tr>
  	      <tr>
  	        <td>CSS</td>
  	      </tr>
  	      <tr>
  	        <td>Javascript</td>
  	      </tr>
          <tr>
            <td onClick = {() => this.linkHandler(<FrontEndServer />)}>Front-end server</td>
          </tr>
  	      <tr>
  	        <td>Bootstrap</td>
  	      </tr>
  	      <tr>
  	        <td onClick = {() => this.linkHandler(<ReactInfo />)}>React</td>
  	      </tr>
  	      <tr>
  	        <th>Back end</th>
  	      </tr>
  	      <tr>
  	        <td>Javascript</td>
  	      </tr>
  	      <tr>
  	        <td onClick = {() => this.linkHandler(<NodeInfo />)}>Node.js</td>
  	      </tr>
          <tr>
            <td onClick = {() => this.linkHandler(<BackEndServer />)}>Back-end server</td>
          </tr>
          <tr>
            <th>Database Management</th>
          </tr>
          <tr>
            <td onClick = {() => this.linkHandler(<Mongo />)}>MongoDB</td>
          </tr>
  	      <tr>
  	        <th>Functionality</th>
  	      </tr>
  	      <tr>
  	        <td onClick = {() => this.linkHandler()}>Website search</td>
  	      </tr>
  	      <tr>
  	        <td onClick = {() => this.linkHandler(<Captcha />)}>Captcha</td>
  	      </tr>
          <tr>
            <td>Email</td>
          </tr>
          <tr>
            <td onClick = {() => this.linkHandler(<Map />)}>Google Maps</td>
          </tr>
  	      <tr>
  	        <th>Cloud Computing</th>
  	      </tr>
  	      <tr>
  	        <td onClick = {() => this.linkHandler(<Cloud />)}>Amazon EC2</td>
  	      </tr>
  	      <tr id="lol">
  	        <td onClick = {() => this.linkHandler(<Storage />)}>Amazon S3</td>
  	      </tr>
  	    </table>

        <div class="stackContent">
          {this.state.form}
        </div>

  	  </div>
  	)
  }
}

export default Stack;