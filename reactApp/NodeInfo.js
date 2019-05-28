import React from 'react';
import ReactDOM from 'react-dom';
import './stack.css';

class NodeInfo extends React.Component {
  constructor(props) {
    super(props);
    this.state = { } 
  }
  render() {
    return (
      <div class="info">
        <h1>Node.js</h1>
        <p>Node.js is an open-source server environment which runs code outside of the web browser.</p>
        <p>Node.js allows developers to efficiently create web applications, servers, API's and libraries.</p>
        <p>Node.js is asynchronous which means higher performance and speed for my website; as soon as my back-end server receives a request it is ready to handle another.</p>
        <p>I implemented my back-end server in the Node.js environment to efficiently handle all requests coming from the client (i.e. you).</p>
        <p>Node.js is highly portable and runs on Windows, MacOS, Unix and Linux.</p>
        <a href="https://nodejs.org/en/">Click here for further documentation</a>
      </div>
    )
  }
}

export default NodeInfo;