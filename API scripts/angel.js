var http = require('http');
var fs = require('fs');
var json2csv = require('json2csv');


callback = function(response) {

  	var str = '';
  	
  	response.setEncoding('utf8');
  
  	//another chunk of data has been recieved, so append it to `str`
  	response.on('data', function (chunk) {
		str += chunk;
  	});

  	//the whole response has been recieved, so we just print it out here
  	response.on('end', function () {
		
		var result = JSON.parse(str);
	 	var startups = result['startups'];

 
		//specify the keys wanted in csv	
		var keys = [ 'name', 'launch_date', 'locations','company_size', 'logo_url'];

		//iterate through all the startups returned for this page
		for(var j =0; j<startups.length;j++){
	
			var info = startups[j];
	
			if(info != undefined && info != null){
				// I guess otherwise the formatting of the data is bad.
				if(info.hasOwnProperty('locations')){
					var temp = info['locations'];
			
					//a common problem - startups have null locations array
					if(temp[0] != undefined && temp[0] != null){
						var loc = '';
				
						for(var k = 0; k<temp.length; k++){
							if(temp[k].hasOwnProperty('display_name')) loc += temp[k]['display_name']+';';
						}
						info['locations']=loc;
					}
					
					//convert json data into csv
					//I print it on the console and from terminal I write the output to a .csv file
					json2csv({data: info , fields: keys, hasCSVColumnTitle: false }, function(err, csv) {
						if (err) console.log(err);
						console.log(csv);
					});
				}
			}
		}
	  });
}	  

//for now, the number of pages is hardcoded.
numPages = 86;	

for(var i = 1; i<numPages+1; i++){
	
	var options = {
		hostname: 'api.angel.co',
		path: '/1/startups?filter=raising&page='+i,
		method: 'GET'
	};
	
	
	var req = http.request(options, callback);
	
		
	req.on('error', function(e) {
	  console.log('problem with request: ' + e.message);
	});

	req.end();
}
