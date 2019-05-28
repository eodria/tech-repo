import React from 'react';
import ReactDOM from 'react-dom';
import './stack.css';

class Storage extends React.Component {
  constructor(props) {
    super(props);
    this.state = { }
  }
  render() {
    return (
      <div class="info">
        <h1>Amazon Simple Storage Service</h1>
        <p>Amazon S3 is a simple data storage service which is highly scalable.</p>
        <p>Amazon S3 uses the same infrustructure that Amazon uses for its e-commerce market.</p>
        <p>Developers and companies can save major costs since you only pay what you use for.</p>
        <p>It also is efficient to integrate Amazon S3 with software applications since it reduces memory usage.</p>
        <p>I used Amazon S3 to store all the pictures and logos you see on my website.</p>
        <a href="#">Click here for further documentation</a>
      </div>
    )
  }
}

export default Storage;