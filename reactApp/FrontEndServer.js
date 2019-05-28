import React from 'react';
import ReactDOM from 'react-dom';
import './stack.css';

class FrontEndServer extends React.Component {
  constructor(props) {
    super(props);
    this.state = { }
  }
  render() {
    return (
      <div class="info">
        <h1>Front-end server</h1>
        <p>The front-end server is the component with which the client (i.e. you) directly communicate with.</p>
        <p>The front-end server forwards all requests to the back-end server and then forwards the response back to you.</p>
        <p>This is how the majority of web applications are created.</p>
        <p>This ensures equal load balancing and enhanced efficiency/throughput.</p>
      </div>
    )
  }
}

export default FrontEndServer;