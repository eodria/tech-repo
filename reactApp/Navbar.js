import React from 'react';
import ReactDOM from 'react-dom';
import './siteStyle.css';
import $ from 'jquery';
import MatchInfo from './MatchInfo.js';
import Modal from 'react-modal';

class Navbar extends React.Component {
  constructor(props) {
    super(props);
    this.state = { };
    this.openModal = props.openModal.bind(this);
  }
	render() {
		return (
        <div>

		    <nav name="header" class="navbar navbar-expand-lg navbar-light bg-light">
              <a name="home" class="navbar-brand" href="/">Home</a>
              <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbarSupportedContent" aria-controls="navbarSupportedContent" aria-expanded="false" aria-label="Toggle navigation">
                <span class="navbar-toggler-icon"></span>
              </button>

              <div class="collapse navbar-collapse" id="navbarSupportedContent">
                <ul name="navigate" class="navbar-nav mr-auto" id="nav-list">
                  <li name="nested" class="nav-item active">
                    <a class="nav-link" href="stack.html">About</a>
                  </li>
                  <li name="nested" class="nav-item active">
                    <a class="nav-link" href="resume.html">Resume<span class="sr-only">(current)</span></a>
                  </li>
                  <li name="nested" class="nav-item active">
                    <a class="nav-link" href="https://www.chegg.com/tutors/online-tutors/Estevon-O-543280/">Chegg Tutors</a>
                  </li>
                  <li name="nested" class="nav-item active">
                    <a class="nav-link" href="contact.html">Contact</a>
                  </li>
                  <li name="nested" name="search" class="nav-item active">
                    <span name="search">
                      <button onClick={this.openModal} name="search" type="button">
                        <a name="search" class="nav-link" href="#">Search</a>
                      </button>
                      <i class="fa fa-search"></i>
                    </span>
                  </li>
                </ul>

              </div>
        </nav>

      </div>
		);
	}
}
export default Navbar;