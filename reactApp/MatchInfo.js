import React from 'react';
import ReactDOM from 'react-dom';
import './stack.css';
import Stack from './Stack.js';

class MatchInfo extends React.Component {
  constructor(props) {
    super(props);
    this.state = { 
      render: false,
      length: 0,
      text: ''
    }
  }
  render() {
    const render = this.props.render;
    let textToBeRendered;
    if (render) {
      textToBeRendered = this.props.text;
    }
    else {
      textToBeRendered = '';
    }
    return (
      <div>
        {textToBeRendered}
      </div>
    )
  }
}

export default MatchInfo;