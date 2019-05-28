import React, { Component } from 'react';
import { Map, GoogleApiWrapper } from 'google-maps-react';
import { InfoWindow, Marker } from 'google-maps-react';

const mapStyles = {
  width: '90%',
  height: '90%'
};

function buildMap() {
  var map = { };
  map['Schaumburg'] = 'I interned as a Technology Development Intern at ' +
            'Optum, a subsidiary of UnitedHealth Group. The program was ten weeks in length. I temporarily ' +
            'relocated to Schaumburg, a northwestern suburb of Chicago, Illinois.';
  map['Pittsburgh'] = 'I attended my undergraduate at Carnegie Mellon University located in ' +
            'Pittsburgh, PA. I received my Bachelor of Science in Electrical and Computer Engineering in May 2018.';
  map['San Francisco'] = 'I currently live and work in San Francisco. I interned at Esurance in the summer of ' +
            '2018 as a Software Engineering Intern. I then shortly received and accepted an offer for a full-time software engineer position.';
  map['Miami'] = 'I was born here!';
  map['Memphis'] = 'I lived my entire childhood here before leaving for university';
  return map;
}

export class MapContainer extends Component {
  state = {
    showingInfoWindow: false,
    activeMarker: {},
    selectedPlace: {},
    markerStr: null,
    contentStr: null,
    map: buildMap()
  };
  onMarkerClick = (props, marker, e) => {
    this.setState({
      selectedPlace: props,
      activeMarker: marker,
      showingInfoWindow: true,
    }, () => {
      this.setState({ contentStr: this.state.map[this.state.selectedPlace.name] });
    });
  };
  onClose = props => {
    if (this.state.showingInfoWindow) {
      this.setState({
        showingInfoWindow: false,
        activeMarker: null
      });
    }
  };
  render() {
    return (
      <Map 
        google={this.props.google} 
        zoom={3.5}
        style={mapStyles}
        initialCenter={{lat: 42.0334, lng: -88.0834}}
      >

        <Marker
          onClick={this.onMarkerClick}
          name={'Schaumburg'}
          position={{lat: 42.0334, lng: -88.0834}}
        />
        
        <Marker
          onClick={this.onMarkerClick}
          name={'Pittsburgh'}
          position={{lat: 40.440624, lng: -79.995888}}
        />

        <Marker
          onClick={this.onMarkerClick}
          name={'San Francisco'}
          position={{lat: 37.773972, lng: -122.431297}}
        />

        <Marker
          onClick={this.onMarkerClick}
          name={'Memphis'}
          position={{lat: 35.1172, lng: -89.9693 }}
        />

        <Marker 
          onClick={this.onMarkerClick}
          name={'Miami'}
          position={{lat: 25.7617, lng: -80.1918 }}
        />

        <InfoWindow
          marker={this.state.activeMarker}
          visible={this.state.showingInfoWindow}
          onClose={this.onClose}
        >

          <div>
            <p>{this.state.contentStr}</p>
          </div>
        
        </InfoWindow>
        
      </Map>
    );
  }
}

export default GoogleApiWrapper({
	apiKey: 'AIzaSyAaoBpPc2lctiQYNaWkUKHdyc0wm7mTByI' 
})(MapContainer);