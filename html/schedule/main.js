let exampleList = [
  {"mode":"timer","m":10,"func":"sendEnvData"},
  {"mode":"daily","h":0,"m":0,"s":0,"func":"fan","params":{"val":255}},
  {"mode":"daily","h":8,"m":0,"s":0,"func":"fan","params":{"val":160}},
  {"mode":"daily","h":18,"m":0,"s":0,"func":"fan","params":{"val":32}}
];

window.addEventListener('load', setup);

var currentTab = "dataTable_timers";

var functionDefs = [];
var functionNames = [];
var shortDows = [];

function setup() {

  getFile("schedule/getFunctionNames", function(contents1) {
    console.log(contents1);
    functionDefs = JSON.parse(contents1);
    functionNames = Object.keys(functionDefs);
    console.log("functions:");
    console.log(functionNames);
    getFile("schedule/getShortDows", function(contents2) {
      console.log(contents2);
      shortDows = JSON.parse(contents2);
    
      loadSchedules();
    }, function () { setState("error cannot load short DOW list"); });
  }, function() { setState("error cannot load function list"); });
  //setupWebSocket();
}

function SpecialJsonStringify(obj) {
  return JSON.stringify(obj, (key, value) => {
    if (key == "params") {
      if (value != "")
        return JSON.parse(value);
      else
        return undefined;
    }
    else
      return value;
  });
}

function getTableRowData_YMD_html(data) {
  let html = "";
  html += '<td class="centered-cell">'+ getTextInputHtml('', 'txtBoxYear center-text', data.y?data.y:2024) +'</td>';
  html += '<td class="centered-cell">'+ getTextInputHtml('', 'txtBoxHMS center-text', data.M?data.M:1) +'</td>';
  html += '<td class="centered-cell">'+ getTextInputHtml('', 'txtBoxHMS center-text', data.d?data.d:1) +'</td>';
  return html;
}

function getTableRowData_HMS_html(data) {
  let html = "";
  html += '<td class="centered-cell">'+ getTextInputHtml('', 'txtBoxHMS center-text', data.h?data.h:0) +'</td>';
  html += '<td class="centered-cell">'+ getTextInputHtml('', 'txtBoxHMS center-text', data.m?data.m:0) +'</td>';
  html += '<td class="centered-cell">'+ getTextInputHtml('', 'txtBoxHMS center-text', data.s?data.s:0) +'</td>';
  return html;
}

function getTableRowData_Func_Params_RemoveBtn_html(data) {
  let html = "";
  html += '<td class="centered-cell">'+ getSelectInputHtml('', 'selectBox center-text', functionNames, data.func) +'</td>';
  html += '<td>'+ getTextInputHtml('', 'txtBoxParams', data.params) + '</td>';
  html += '<td>' + getRemoveButtonHtml() + '</td>';
  return html;
}

function getTableRow_timers_or_daily_html(data) {
  let html = '<tr>';
  html += getTableRowData_HMS_html(data);
  html += getTableRowData_Func_Params_RemoveBtn_html(data);
  html += '</tr>';
  return html;
}

function getTableRow_weekly(data) {
  let html = '<tr>';
  html += '<td class="centered-cell">'+ getSelectInputHtml('', 'txtBoxDOW center-text', shortDows, data.D?data.D:shortDows[0]) +'</td>';
  html += getTableRowData_HMS_html(data);
  html += getTableRowData_Func_Params_RemoveBtn_html(data);
  html += '</tr>';
  return html;
}

function getTableRow_explicit(data) {
  let html = '<tr>';
  html += getTableRowData_YMD_html(data);
  html += getTableRowData_HMS_html(data);
  html += getTableRowData_Func_Params_RemoveBtn_html(data);
  html += '</tr>';
  return html;
}

function drawTables(data) {
  let html_dt_timers = "";
  let html_dt_daily = "";
  let html_dt_weekly = "";
  let html_dt_explicit = "";
  
  for (let i=0;i<data.length;i++)
  {
    if (data[i].params == undefined) data[i].params = "";
    else data[i].params = JSON.stringify(data[i].params); // this needs to be edited as a string

    if (data[i].mode == "timer")
      html_dt_timers += getTableRow_timers_or_daily_html(data[i]);
    else if (data[i].mode == "daily")
      html_dt_daily += getTableRow_timers_or_daily_html(data[i]);
    else if (data[i].mode == "weekly")
      html_dt_weekly += getTableRow_weekly(data[i]);
    else if (data[i].mode == "explicit")
      html_dt_explicit += getTableRow_explicit(data[i]);
  }
  
  document.getElementById("dataTableBody_timers").innerHTML = html_dt_timers;
  document.getElementById("dataTableBody_daily").innerHTML = html_dt_daily;
  document.getElementById("dataTableBody_weekly").innerHTML = html_dt_weekly;
  document.getElementById("dataTableBody_explicit").innerHTML = html_dt_explicit;
}

function addNewItem()
{
  if (currentTab == "dataTable_timers") {
    let html = getTableRow_timers_or_daily_html({m:0,h:0,s:0,func:functionNames[0],params:""});
    document.getElementById("dataTableBody_timers").innerHTML += html;
  }
  else if (currentTab == "dataTable_daily") {
    let html = getTableRow_timers_or_daily_html({m:0,h:0,s:0,func:functionNames[0],params:""});
    document.getElementById("dataTableBody_daily").innerHTML += html;
  }
  else if (currentTab == "dataTable_weekly") {
    let html = getTableRow_weekly({D:shortDows[0],m:0,h:0,s:0,func:functionNames[0],params:""});
    document.getElementById("dataTableBody_weekly").innerHTML += html;
  }
  else if (currentTab == "dataTable_explicit") {
    let html = getTableRow_explicit({y:2024,M:1,d:1,m:0,h:0,s:0,func:functionNames[0],params:""});
    document.getElementById("dataTableBody_explicit").innerHTML += html;
  }
}

function removeItem(element, index, dtName)
{
  console.log(element.parentNode.parentNode);
  
  if (currentTab == "dataTable_timers") {
    var confirmed = confirm("Are you sure you want to remove the item:\n" + SpecialJsonStringify(getTimersOrDailyData(element.parentNode.parentNode)));
    if (confirmed == false) return;
    element.parentNode.parentNode.remove();
  }
  else if (currentTab == "dataTable_daily") {
    var confirmed = confirm("Are you sure you want to remove the item:\n" + SpecialJsonStringify(getTimersOrDailyData(element.parentNode.parentNode)));
    if (confirmed == false) return;
    element.parentNode.parentNode.remove();
  }
  else if (currentTab == "dataTable_weekly") {
    var confirmed = confirm("Are you sure you want to remove the item:\n" + SpecialJsonStringify(getWeeklyData(element.parentNode.parentNode)));
    if (confirmed == false) return;
    element.parentNode.parentNode.remove();
  }
  else if (currentTab == "dataTable_explicit") {
    var confirmed = confirm("Are you sure you want to remove the item:\n" + SpecialJsonStringify(getExplicitData(element.parentNode.parentNode)));
    if (confirmed == false) return;
    element.parentNode.parentNode.remove();
  }
}

function loadSchedules()
{
  getFile("schedule/list.json", 
    function(contents){
      console.log(contents);
      let data = JSON.parse(contents);
      drawTables(data);
    },
    function() { setState("error cannot load schedule/list.json"); });
}

function getTimersOrDailyData(dataRow, mode) {
  let cells = dataRow.getElementsByTagName("td");
  return {
    mode:mode,
    h:parseInt(cells[0].children[0].value),
    m:parseInt(cells[1].children[0].value),
    s:parseInt(cells[2].children[0].value),
    func:cells[3].children[0].value,
    params:cells[4].children[0].value
  };
}

function getWeeklyData(dataRow) {
  console.log(dataRow);
  let cells = dataRow.getElementsByTagName("td");
  return {
    mode:"weekly",
    D:cells[0].children[0].value,
    h:parseInt(cells[1].children[0].value),
    m:parseInt(cells[2].children[0].value),
    s:parseInt(cells[3].children[0].value),
    func:cells[4].children[0].value,
    params:cells[5].children[0].value
  };
}

function getExplicitData(dataRow) {
  let cells = dataRow.getElementsByTagName("td");
  return {
    mode:"explicit",
    y:parseInt(cells[0].children[0].value),
    M:parseInt(cells[1].children[0].value),
    d:parseInt(cells[2].children[0].value),
    h:parseInt(cells[3].children[0].value),
    m:parseInt(cells[4].children[0].value),
    s:parseInt(cells[5].children[0].value),
    func:cells[6].children[0].value,
    params:cells[7].children[0].value
  };
}

function saveSchedules()
{
  let data = [], rows;
  // get timer items
  rows = document.getElementById("dataTableBody_timers").getElementsByTagName("tr");
  for (let i=0;i<rows.length;i++) {
    let newItem = getTimersOrDailyData(rows[i], "timer");
    
    if(newItem.h == 0) newItem.h = undefined;
    if(newItem.m == 0) newItem.m = undefined;
    if(newItem.s == 0) newItem.s = undefined;
    
    if (newItem.h == undefined && newItem.m == undefined && newItem.s == undefined) newItem.s = 1; // at least one second
    
    data.push(newItem);
  }
  // get daily items
  rows = document.getElementById("dataTableBody_daily").getElementsByTagName("tr");
  for (let i=0;i<rows.length;i++) {
    let newItem = getTimersOrDailyData(rows[i], "daily");
    
    // can skip both minutes and second if they are zero, hour is allways specified
    if(newItem.m == 0) newItem.m = undefined;
    if(newItem.s == 0) newItem.s = undefined;
    data.push(newItem);
  }
  // get weekly items
  rows = document.getElementById("dataTableBody_weekly").getElementsByTagName("tr");
  for (let i=0;i<rows.length;i++) {
    let newItem = getWeeklyData(rows[i]);

    // can skip both minutes and second if they are zero, hour is allways specified
    if(newItem.m == 0) newItem.m = undefined;
    if(newItem.s == 0) newItem.s = undefined;
    data.push(newItem);
  }
  // get explicit items
  rows = document.getElementById("dataTableBody_explicit").getElementsByTagName("tr");
  for (let i=0;i<rows.length;i++) {
    let newItem = getExplicitData(rows[i]);
    
    // can skip both minutes and second if they are zero, hour is allways specified
    if(newItem.m == 0) newItem.m = undefined;
    if(newItem.s == 0) newItem.s = undefined;
    data.push(newItem);
  }

  var dataJSON = "[\n";
  for (let i=0;i<data.length;i++) {
    dataJSON += "  " + SpecialJsonStringify(data[i])
    if (i<(data.length-1)) dataJSON += ',';
    dataJSON += "\n";
  }
  dataJSON += "]";
  console.log(dataJSON);
  
  
  postFile("schedule/list.json", dataJSON, "application/json", newScheduleFile_Posted, newScheduleFile_NotPosted)
}

function newScheduleFile_Posted(){
  setState("file saved");
  getFile("/schedule/refresh", function(msg) { setState(msg);}, function() {setState("fail to load new schedule");});
}

function newScheduleFile_NotPosted() {
  setState("fail to save file");
}

