import React from 'react';
import ReactDOM from 'react-dom';
import './siteStyle.css';
import Navbar from './Navbar.js';


// import subcomponents
import Intro from './intro.js';
//import ReactInfo from './ReactInfo.js';
import BackEndServer from './BackEndServer.js';
import BootstrapInfo from './BootstrapInfo.js';

import Captcha from './Captcha.js'; // centered
import Cloud from './Cloud.js'; // centered
import Twilio from './Twilio.js'; // axe
import ReactInfo from './ReactInfo.js'; // centered
import Storage from './Storage.js'; // centered
import Map from './Map.js'; // centered
import NodeInfo from './NodeInfo.js'; // centered
import FrontEndServer from './FrontEndServer.js';
import Mongo from './Mongo.js';

import Stack from './Stack.js';
import MatchInfo from './MatchInfo.js';

import Modal from 'react-modal';


const customStyles = {
  content : {
    top                   : '40%',
    left                  : '50%',
    right                 : '50%',
    bottom                : 'auto',
    marginRight           : '-50%',
    transform             : 'translate(-50%, -50%)'
  }
};

Modal.setAppElement(document.getElementById('app'));

class App extends React.Component {
    constructor(props) {
      super(props);
	    this.state = {
        data: '',
        longitude: 0,
        latitude: 0,
        form: <Intro />,
        map: null,
        modalIsOpen: false,

        // for searching website
        matches: [ ],
        searchToken: '',
        matchSentence: ''
      };
      this.updateState = this.updateState.bind(this);
      this.clearInput = this.clearInput.bind(this);
      this.myInput = React.createRef();
      this.sendSMS = this.sendSMS.bind(this);
      this.getUserLocation = this.getUserLocation.bind(this);
      this.connectToServer = this.connectToServer.bind(this);
      this.redirectionHandler = this.redirectionHandler.bind(this);

      // for modal
      this.openModal = this.openModal.bind(this);
      this.closeModal = this.closeModal.bind(this);

      this.searchHandler = this.searchHandler.bind(this);
      this.tokenHandler = this.tokenHandler.bind(this);
    };
    redirectionHandler(form) { 
      console.log('redirecting... ');
      var updatedForm = this.state.map[form];
      console.log('form: ' + form);
      console.log('updatedForm: ' + updatedForm);
      this.setState({ form: updatedForm, modalIsOpen: false });
    }
    getUserLocation() {
      var lat = 0;
      var long = 0;
      navigator.geolocation.getCurrentPosition(function(position) {
        lat = position.coords.latitude;
        long = position.coords.longitude;
      });
      this.setState({ longitude: long, latitude: lat });
    }
    connectToServer() {
      fetch('/');
    }
    // construct the map
    // make these subcomponents contain the sidebar...
    componentDidMount() {
      var mapping = {
        'Home Page': <Intro />,
        'React': <Stack form=<ReactInfo /> />,
        'Bootstrap': <Stack form=<BootstrapInfo/> />,
        'Captcha': <Stack form=<Captcha /> />,
        'Storage': <Stack form=<Storage /> />,
        'Map': <Stack form=<Map /> />,
        'Node': <Stack form=<NodeInfo /> />,
        'Frontend': <Stack form=<FrontEndServer /> />,
        'Mongo': <Stack form=<Mongo /> />
      };
      this.setState({ map: mapping });
    }
    updateState(e) {
      this.setState({ data: e.target.value });
    }
    sendSMS() {
      fetch('http://13.57.47.126:8081/sendSMS', {
        body: JSON.stringify(this.state),
        cache: 'no-cache',
        credentials: 'same-origin',
        headers: {
          'content-type': 'application/json'
        },
        method: 'POST',
        mode: 'cors',
        redirect: 'follow',
        referrer: 'no-referrer'
      })
      .then(function(response) {
        console.log(response);
        return response.json();
      })
      .then(function(myJson) {
        console.log(myJson);
      });
    }
    clearInput() {
      this.setState( { data: '' } );
      ReactDOM.findDOMNode(this.refs.myInput).focus();
    }
    openModal() {
      this.setState({ modalIsOpen: true });
    }
    closeModal() {
      this.setState({ modalIsOpen: false });
    }
    afterOpenModal() {
      this.subtitle.style.color = '#f00';
    }
    searchHandler() {
      console.log('Initializing search for token... ');
      //fetch('http://52.52.115.98:8081/search', {
      fetch('http://localhost:8081/search', {
        body: JSON.stringify(this.state),
        cache: 'no-cache',
        credentials: 'same-origin',
        headers: {
          'content-type': 'application/json'
        },
        method: 'POST',
        mode: 'cors',
        redirect: 'follow',
        referrer: 'no-referrer'
      })
      .then(function(response) {
        console.log(response);
        var myJson = response.json();
        return myJson;
      })
      .then(myJson => {
        this.setState({ matches : myJson });
        console.log(myJson);
        $('#matches').html(myJson.length + ' matches found: ');
      });
    }
    tokenHandler(e) {
      this.setState({ searchToken: event.target.value });
    }
    render() {

      var text = "Matches found: " + this.state.matches.length;
      var listItems = this.state.matches.map((d) => <li class="match-item"><a class="matchHeader" onClick = {() => this.redirectionHandler(d.title)}>{d.title}</a><br/>{d.sentence}</li>);
  

      return (
    
      	<div class="parent">
        
        <Navbar redirectionHandler={this.redirectionHandler} 
                closeModal={this.closeModal}
                openModal={this.openModal}
                afterOpenModal={this.afterOpenModal} />
        
        {this.state.form}

         <Modal
          isOpen={this.state.modalIsOpen}
          onAfterOpen={this.afterOpen}
          onRequestClose={this.closeModal}
          style={customStyles}
          contentLabel="Example Modal"
          > 
            <div class="modal-header">
              <h4 class="modal-title">Search website</h4>
                <button type="button" class="close" onClick={this.closeModal}>&times;</button>
              </div>
                          
              <div class="modal-body">
                <div name="query" class="form-group">
                  <input onChange={this.tokenHandler} id="search-box" class="form-control form-rounded" aria-describedby="nameHelp" placeholder="Enter search token">
                  </input>

                  {<MatchInfo render={this.state.matches.length > 0} text={text} /> }
                  {listItems}

                </div>
              </div>

              <div class="modal-footer">
                <button onClick={this.closeModal} type="button" name="close" class="btn btn-danger" data-dismiss="modal">Close</button>
                <button onClick={this.searchHandler} name="submit" type="button" class="btn btn-success">Search</button>
              </div>

          </Modal>
      
        </div>

      );
   }
}

export default App;