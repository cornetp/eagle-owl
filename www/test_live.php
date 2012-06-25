<!doctype html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Demo</title>
    <script src="jquery-flot/jquery.js"></script>
    <script language="javascript" type="text/javascript" src="jquery-flot/jquery.flot.js"></script>
<!--    <script type="text/javascript" src="date.js"></script> -->
    <script type="text/javascript" src="datejs/build/date-fr-FR.js"></script>
  </head>
  <body>
    <h1>Flot Examples</h1>

    <div id="live_graph" style="width:600px;height:300px;"></div>
    <p>Time between updates: <input id="updateInterval" type="text" value="" style="text-align: right; width:5em"> milliseconds</p>

    <script type="text/javascript">
      $(function () 
      {
        var graph = $("#live_graph");
        var data = [], totalPoints = 300;
        var marks = [];
        function getData() 
        {
          // init with 0
          if(data.length == 0)
          {
            for(var i = 0; i < totalPoints; ++i)
              data.push(0);
          }
          // remove the oldest value
          if(data.length >= totalPoints)
            data = data.slice(1);
       
          if(typeof getData.counter == 'undefined' )
              getData.counter = 0;
 
          function onNewData(new_data)
          {
            if(typeof onNewData.prev == 'undefined' )
              onNewData.prev = 0;

            getData.counter++;
            var v = parseFloat(new_data);
            data.push(v);
            if(getData.counter == 60/*v == 10 && onNewDataprev != 10*/)
            {
              var mark = [];
              mark['pos'] = totalPoints;
              mark['text'] = 'hh:mm:ss';
              marks.push(mark);
              getData.counter = 0;
            }
            onNewData.prev = v;
          }

          // add the new one
          $.ajax({
            url: "live.php",
            method: 'GET',
            dataType: 'text',
            success: onNewData
          });
     
          // update marks
          var num_marks = marks.length;
          for(var i = 0; i < num_marks; ++i)
            --marks[i].pos;

          if(num_marks > 0 && marks[0].pos <= 0)
            marks = marks.slice(1);
 
          // zip the generated y values with the x values
          var res = [];
          for(var i = 0; i < data.length; ++i)
            res.push([i, data[i]])
          return res;
        }
      
        function markings() 
        {
          var markings = [];
          for(var i = 0; i < marks.length; i++)
          {
            var mark = marks[i];
            if(marks[i].pos > 1)
              markings.push({ color: '#000', lineWidth: 1, xaxis: { from: marks[i].pos, to: marks[i].pos } });
          }
          return markings;
        }

        // setup control widget
        var updateInterval = 1000;
        $("#updateInterval").val(updateInterval).change(function () 
        {
          var v = $(this).val();
          if (v && !isNaN(+v)) {
            updateInterval = +v;
            if (updateInterval < 500)
              updateInterval = 500;
            if (updateInterval > 10000)
              updateInterval = 10000;
            $(this).val("" + updateInterval);
          }
        });
      
        // setup plot
        var options = {
            series: { shadowSize: 0 }, // drawing is faster without shadows
            yaxis: { min: 0, autoscaleMargin: 0.01 },
            //xaxis: { show: false }
            xaxis: { mode: "time",timeformat: "%H:%M"},
            grid: { markings: markings }
        };
        var plot = $.plot(graph, [ getData() ], options);
      
        function update() {
            plot.setData([ getData() ]);
/*
            var num_marks = marks.length;
            for(var i = 0; i < num_marks; ++i)
            {
              var mark = marks[i];
              var str = '<div style="position:absolute;left:' + 
                          (mark['pos'] + 4) + 'px;top:50' + 
                          'px;color:#666;font-size:smaller">' +
                          mark['text'] + '</div>';
              graph.append(str);
            }*/
   
            plot.setupGrid();
            plot.draw();
            
            setTimeout(update, updateInterval);
        }
      
        update();
      });
    </script>
  </body>
</html>
