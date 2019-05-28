import React from 'react';
import ReactDOM from 'react-dom';
import './stack.css';

class BackEndServer extends React.Component {
  constructor(props) {
    super(props);
    this.state = { } 
  }
  render() {
    return (
      <div class="info">
        <h1>Back-end server</h1>
        <p>I connected my front-end server to the back-end server of my web application.</p>
        <p>This back-end server is responsible for many things such as: </p>
        <ul>
          <li>Captcha verification requests (proving you are not a robot =] )</li>
          <li>Email and phone number validation on Contact page</li>
          <li>Email functionality</li>
          <li>Serving all the pages and content to the client (i.e. you)</li>
          <li>Website search functionality</li>
          <li>Database management</li>
        </ul>
      </div>
    )
  }
}

export default BackEndServer;