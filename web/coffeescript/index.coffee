$ ->
  $("#analyzerOutput").hide()
  $("#analyzeButton").click ->
    input = $("#inputText").val()
    if input.length == 0
      input = "Here is a sentence to analyze."
    if input.length > (512 * 1024)
      input = "Try using a smaller amount of text."
    $("#analyzerOutput").show()
    $.ajax "http://localhost:9001/meta-nlp-api",
      type: "POST"
      dataType: "text"
      data: input
      success: (data, stat, xhr) -> printSentences JSON.parse(data)
      failure: (axhr, stat, err) -> console.log "Failed!"

printSentences = (result) ->
  window.DT.destroy() if window.DT isnt undefined # clear old data
  $("#sentenceList tbody").html('') # clear old data
  totalSentences = 0
  for elem in result.sentences
    ++totalSentences
    html = "<tr><td>#{totalSentences}</td><td>#{elem.sentence}</td></tr>"
    $("#sentenceList tbody").append(html)
  window.DT = $("#sentenceList").DataTable({"bPaginate": false, "bInfo": false})
