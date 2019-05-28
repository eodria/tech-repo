import React from 'react';
import ReactDOM from 'react-dom';
import './stack.css';
import Stack from './Stack.js';

class Cloud extends React.Component {
  constructor(props) {
    super(props);
    this.state = { } 
  }
  render() {
    return (
      <div class="info">
        <h1>Amazon Elastic Cloud Compute</h1>
        <p>Amazon EC2 (Elastic Compute Cloud) is a web service which allows scalable cloud computing capacity.</p>
        <p>I rented and configured my own virtual servers/computers to host both the client-side and server-side applications for the production of my website.</p>
        <p>Cloud computing is extremely efficient for businesses for several reasons.</p>
        <p>This allows companies to simply rent out computing power and scale as needed.</p>
        <p>The need for physical computers/servers is completely eliminated.</p>
        <a href="https://aws.amazon.com/ec2/">Click here for further documentation</a>
      </div>
    )
  }
}

export default Cloud;