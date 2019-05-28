import React from 'react';
import ReactDOM from 'react-dom';
import $ from 'jquery';
import './stack.css';

class ReactInfo extends React.Component {
  constructor(props) {
    super(props);
    this.state = { }
  }
  render() {
    return (
      <div class="info">
        <h1>React</h1>
        <p>React is a JavaScript library used to build user interfaces.</p>
        <p>It was created by Facebook and is currently one of the hottest front-end tools in web development today.</p>
        <p>It efficiently re-renders changes on websites whenever state/data changes.</p>
        <p>Since React works in terms of components, engineers can quickly and efficiently build their user interfaces.</p>
        <p>The entire front-end of my website is made with React.</p>
        <p>I also use React in my full-time job to connect my back-end services.</p>
        <a href="https://reactjs.org/">Click here for further documentation</a>
      </div>
    )
  }
}

export default ReactInfo;