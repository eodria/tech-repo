import React from 'react';
import ReactDOM from 'react-dom';
import './stack.css';

class WebsiteSearch extends React.Component {
  constructor(props) {
    super(props);
    this.state = { }
  }
  render() {
    return (
      <div>
        <h1>Website Search</h1>
        <p>You are able to search text content of my website by clicking on the upper right-hand corner of this website.</p>
        <p>The back-end server stores all my pages as well as their text.</p>
        <p>The search request is handled asynchronously.</p>
      </div>
    )
  }
}

export default WebsiteSearch;