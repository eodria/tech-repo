import React from 'react';
import ReactDOM from 'react-dom';
import './stack.css';

class Map extends React.Component {
  constructor(props) {
    super(props);
    this.state = { }
  }
  render() {
    return (
      <div class="info">
        <h1>Google Maps API</h1>
        <p>Google offers a Google Maps software library which most of certainly use today.</p>
        <p>I integrated Google Maps on my Resume page to offer a visualization of where I have worked and studied.</p>
        <p>Several uses for Google Maps include looking up directions, geolocation and much more.</p>
        <a href="https://developers.google.com/maps/documentation/">Click here for further documentation</a>
      </div>
    )
  }
}

export default Map;