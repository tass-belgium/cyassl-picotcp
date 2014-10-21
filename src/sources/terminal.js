function focusOnInput()
{
    var input = document.getElementById("terminal_in");
    var output = document.getElementById("terminal_out");
    input.focus();
    input.value = "> ";

    input.addEventListener("keydown", function(e) {
        if (!e) { var e = window.event; }
        if (e.keyCode == 13)
        {
            terminalInput();
            input.value = "> ";
        }
    }, false);

    getTerminalOutput();
}

function getTerminalOutput()
{
    var xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function()
    {
        if (xmlhttp.readyState == 4 && xmlhttp.status == 200)
        {
            var new_output = xmlhttp.responseText;
            if(new_output)
            {
                var output = document.getElementById("terminal_out");
                new_output.replace(/\r\n/g, '<br />');
                new_output.replace(/\n\r/g, '<br />');
                new_output.replace(/\n/g, '<br />');
                new_output.replace(/\r/g, '<br />');
                output.innerHTML = output.innerHTML + new_output;
                output.scrollTop = output.scrollHeight;
            }
            setTimeout(function(){getTerminalOutput()}, 500);
        }
    }
    xmlhttp.open("GET","terminal_output",true);
    xmlhttp.timeout = 20000;
    xmlhttp.ontimeout = function()
    {
        getTerminalOutput();
    }
    xmlhttp.onerror = function(e)
    {
        setTimeout(function(){getTerminalOutput()}, 20000);
    }
    xmlhttp.send();
}

function terminalInput()
{
    var ajaxPost = new XMLHttpRequest;
    ajaxPost.open("POST","terminal_input",true);
    ajaxPost.send("in="+ document.getElementById("terminal_in").value.slice(2) +"\n");
}
