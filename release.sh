git submodule update
make -C cpp/build -j4
if [ $? -ne 0 ]; then
  echo "Compiling API server failed!"
  return 1
fi

make -C web/
if [ $? -ne 0 ]; then
  echo "Compiling coffeescript/sass failed!"
  return 1
fi

cp web/javascript/index.js /srv/www/timan103/html/nlp-demo/javascript/
cp web/css/tagged-text.css /srv/www/timan103/nlp-demo/css/
cp web/index.html /srv/www/timan103/html/nlp-demo/

echo "Success! Don't forget to restart the API server if necessary."
