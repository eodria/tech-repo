const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');

module.exports = {
	entry: './main.js',
	output: {
        path: path.join(__dirname, '/bundle'),
        filename: 'index_bundle.js'
	},
	devServer: {
		inline: true,
		port: 8080
	},
	module: {
		rules: [
		    {
		    	test: /\.jsx?$/,  
		    	exclude: /node_modules/,
		    	loaders: 'babel-loader', 
		    	query: {
		    		presets: ["@babel/preset-env", "@babel/react"]
		    	}
		    },
		    {
		    	test: /\.css$/,
		    	loader: 'style-loader!css-loader'
		    }
		]
	},
	plugins: [
	    new HtmlWebpackPlugin({
	    	template: './index.html'
	    })
	],
	node: {
		fs: "empty"
	}
}