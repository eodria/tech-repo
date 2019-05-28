import React from 'react';
import ReactDOM from 'react-dom';
import $ from 'jquery';
import './siteStyle.css';

class Intro extends React.Component {
  constructor(props) {
    super(props);
    this.state = { }
  }
  render() {
    return (
      <div> 
        
        <h1 class="welcome">Welcome</h1>

        <div class="outer">

          <p>My name is Estevon Odria. I'm a full-time software engineer in San Francisco.</p>
          <p>I am currently interested in distributed systems and cloud computing.</p>
          <p>I've had the pleasure of working with several companies to deliver great technology.</p>
          <p>My hobbies include: anime, videogames, music, investment, software design and</p>
          <p>running. Check out this website I coded from scratch. If you have inquiries, want a </p>
          <p>full version of my resume instantly emailed to you or want to chat with</p>
          <p>me about life, go to the 'Contact' section.</p> 
          
        </div>
      
      </div>
      
    )
  }
}

export default Intro;