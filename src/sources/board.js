var update_speed = 500;

function updateBoardInfo()
{
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function()
    {
        if (xmlhttp.readyState == 4 && xmlhttp.status == 200)
        {
            var info = JSON.parse(xmlhttp.responseText);
            document.getElementById("uptime").innerHTML = "uptime: "+ info.uptime +" seconds";
			document.getElementById("button").src = info.button + ".png"
            document.getElementById("led1_state").style.backgroundColor = (info.l1 == "on") ? "#22FF22" : "#002200";
            document.getElementById("led2_state").style.backgroundColor = (info.l2 == "on") ? "#2222FF" : "#000022";
            document.getElementById("led3_state").style.backgroundColor = (info.l3 == "on") ? "#FFBB00" : "#221100";
            document.getElementById("led4_state").style.backgroundColor = (info.l4 == "on") ? "#FF2222" : "#220000";
            setTimeout(function(){updateBoardInfo();}, update_speed);
        }
    };
    xmlhttp.open("GET","board_info",true);
    xmlhttp.timeout = 3000;
    xmlhttp.ontimeout = function()
    {
        updateBoardInfo();
    }
    xmlhttp.onerror = function(e)
    {
        setTimeout(function(){updateBoardInfo()}, 1000);
    }
    xmlhttp.send();
}

function toggle_led1(){
    var ajaxPost = new XMLHttpRequest;
    ajaxPost.open("GET","led1",true);
    ajaxPost.send("");
}

function toggle_led2(){
    var ajaxPost = new XMLHttpRequest;
    ajaxPost.open("GET","led2",true);
    ajaxPost.send("");
}

function toggle_led3(){
    var ajaxPost = new XMLHttpRequest;
    ajaxPost.open("GET","led3",true);
    ajaxPost.send("");
}

function toggle_led4(){
    var ajaxPost = new XMLHttpRequest;
    ajaxPost.open("GET","led4",true);
    ajaxPost.send("");
}

function getIp()
{
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function()
    {
        if (xmlhttp.readyState == 4 && xmlhttp.status == 200)
        {
            document.getElementById("ip").innerHTML = xmlhttp.responseText;
        }
    }
    xmlhttp.open("GET","ip",true);
    xmlhttp.send();
}
