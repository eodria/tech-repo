import React from 'react';
import ReactDOM from 'react-dom';
import $ from 'jquery';
import './stack.css';

class Email extends React.Component {
  constructor(props) {
    super(props);
    this.state = { } 
  }
  render() {
    return (
      <div>
        <h1>Email functionality</h1>
        <p>I implemented email functionality from scratch for my website.</p>
        <p></p>
        <a href="https://aws.amazon.com/ses/">Click here for further documentation</a>
      </div>
    )
  }
}

export default Email;
