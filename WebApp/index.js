var xValues = [];
var barColors = ["red", "green","blue","yellow","orange","aqua","magenta"];
var yValues1 = [0, 0, 0, 0, 0, 0, 0];
var yValues2 = [0, 0, 0, 0, 0, 0, 0];
var ch1;
var ch2;
    
function publish_topic_to_iotcore(msg){
    var myHeaders = new Headers();
    var raw = JSON.stringify({"message":msg});

    var requestOptions = {
        method: 'POST',
        headers: myHeaders,
        body: raw
    };

    fetch("https://r4wbx6fu3a.execute-api.us-east-1.amazonaws.com/dev", requestOptions)
    .then(response => response.text())
    .then()
}


function compare( a, b ) {
    if ( a.ts < b.ts ){
      return -1;
    }
    if ( a.ts > b.ts ){
      return 1;
    }
    return 0;
  
}

function update_chart_values(d,val){
    data = new Date();
    curr_date = data.getDate();
    month = data.getMonth();
    x = d.getDate() - curr_date +6;
    if(x>6){
        if(month==4 || month==6 || month==9 || month==11){
            x = x - 30;
        }
        else if(month == 2){
            x = x - 28;
        }
        else{
            x = x - 31;
        }
    }
    if(val<= 450){
        ch1.data.datasets[0].data[x] = ch1.data.datasets[0].data[x]+1
    }
    else{
        ch2.data.datasets[0].data[x] = ch2.data.datasets[0].data[x]+1
    } 
    ch1.update();
    ch2.update();

}

function update_chart_labels(date){
    month = date.getMonth()
    day = date.getDay() + 1
    date_month = date.getDate() - 7
    for(let i=0;i<7;i++){
        day_week = (day + i)%7
        
        switch (day_week) {
            case 0:
                d = "Sun";
                break;
            case 1:
                d = "Mon";
                break;
            case 2:
                d = "Tue";
                break;
            case 3:
                d = "Wed";
                break;
            case 4:
                d = "Thu";
                break;
            case 5:
                d = "Fri";
                break;
            case  6:
                d = "Sat";
        }
        date_month = date_month + 1

        if(date_month < 1){
            if(month==4 || month==6 || month==9 || month==11){
                date_m = 30 + date_month
            }
            else if(month == 2){
                date_m = 28 + date_month 
            }
            else{
                date_m = 31 + date_month 
            }
            label = d + " " + (date_m).toString()
        }
        else{
            label = d + " " + (date_month).toString()
        }
        
        ch1.data.labels.push(label);
        
        ch1.update();
        ch2.update();
    }

}

function displayData(data){
    table = document.getElementById("info")
    
    values = []
    data = JSON.parse(data);
    
    for(let i=0;i < data.length; i++){
        entry = {}
        entry["ts"]=data[i]["ts"]["N"]
        entry["alcool_level"]=data[i]["level"]["M"]["message"]["S"]
        values.push(entry)
        
    }
    values.sort(compare)

    current_date=new Date();
    update_chart_labels(current_date);

    for(let i=0;i < values.length; i++){
        date_test = new Date(parseInt(values[i]["ts"]))
        day_week = date_test.getDay()
        day_month = date_test.getDate()
        
        if(day_month != current_date.getDate() && day_week==current_date.getDay()){
            continue;
        }
        else{
            update_chart_values(date_test,values[i]["alcool_level"])

            if(day_week==current_date.getDay()){
                if(date_test.getMinutes()<10){
                    time=date_test.getHours().toString() + ":0" + date_test.getMinutes().toString()
                }
                else{
                    time =date_test.getHours().toString() + ":" + date_test.getMinutes().toString()
                }
                
                row = table.insertRow(1);
                cell1 = row.insertCell(0);
                cell2 = row.insertCell(1);
                cell1.innerHTML = time;
                cell2.innerHTML = values[i]["alcool_level"];
            }
            

        }
   
    }
    
    
}

function callAPI(){
    var myHeaders = new Headers();
    var requestOptions = {
        method: 'GET',
        headers: myHeaders,
    };
    
    fetch("https://r4wbx6fu3a.execute-api.us-east-1.amazonaws.com/dev", requestOptions)
    .then(response => response.text())
    .then(result => displayData(JSON.parse(result).body))
    
}

function createPlot(name, yval){

    ch = new Chart(name, {
        type: "bar",
        data: {
            labels: xValues,
            datasets: [{
            backgroundColor: barColors,
            data: yval
            }]
        },
        options: {
            legend: {display: false},
            title: {
            display: true,
            },
            scales: {
                yAxes: [{
                  ticks: {
                    beginAtZero: true
                  }
                }],
              }
        }
        });
    return ch;


}



function init(){   
    
    ch1=createPlot("Chart1", yValues1);

    ch2=createPlot("Chart2", yValues2);

    callAPI();

}
  