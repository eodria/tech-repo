import React from 'react';
import ReactDOM from 'react-dom';
import './stack.css';

class Mongo extends React.Component {
  constructor(props) {
    super(props);
    this.state = { }
  }
  render() {
    return (
      <div class="info">
        <h1>MongoDB</h1>
        <p>My page text and contents are stored within a Mongo database connected to my back-end server.</p>
        <p>MongoDB is an extremely popular NoSQL database program which uses JSON-like documents to store represent data.</p>
        <p>MongoDB is also what I use at Esurance to store customer payment information in the Digital Wallet application.</p>
        <p>MongoDB has several benefits over SQL datase programs:</p>
        <ul>
          <li>
            Data is stored in JSON-like format which is a popular choice of data representation.
          </li>
          <li>
            MongoDB is incredibly easy to scale for businesses; MongoDB supports horizontal scaling for heavy workloads.
          </li>
          <li>
            MongoDB does not impose a strict schema in comparison to SQL database programs which is often more realistic.
            This gives engineers greater flexibility as to what they can store in a collection.
          </li>
          <li>
            MongoDB is appropriate for: Big Data and User Data Management.
          </li>
        </ul>

        <a href="https://www.mongodb.com/">Click here for further documentation</a>
        
      </div>
    )
  }
}

export default Mongo;