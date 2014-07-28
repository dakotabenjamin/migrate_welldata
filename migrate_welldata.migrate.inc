<?php

/**
 * @file
 *   An importer for well data
 */


/*
 * You must implement hook_migrate_api(), setting the API level to 2, for
 * your migration classes to be recognized by the Migrate module.
 */
function migrate_welldata_migrate_api() {
  $api = array(
    'api' => 2,
    'groups' => array(
      'welldata' => array(
        'title' => t('Well Data'),
        'default_format' => 'filtered_html',
      ),
    ),
    'migrations' => array(
      'WellData' => array(
        'class_name' => 'WellDataMigrate',
        'group_name' => 'welldata',
      ),
    ),
  );
  return $api;
}

/**
 * A migration that is reused for each source CSV file.
 */
class WellDataMigrate extends Migration {
  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import box scores from CSV file.');

    // Create a map object for tracking the relationships between source rows
    $this->map = new MigrateSQLMap($this->machineName,
        array(
          'unkey' => array('type' => 'varchar',
                           'length' => 255,
                           'not null' => TRUE,
                           'description' => 'A unique key in the format [serial]-[datetime]',
                          ),
        ),
        MigrateDestinationNode::getKeySchema()
      );

    // Create a MigrateSource object, which manages retrieving the input data.
    $this->source = new MigrateSourceCSV($arguments['source_file'], $this->csvcolumns(), array('header_rows'=>6), $this->fields());

    $this->destination = new MigrateDestinationEntity('migrate_welldata');

    $this->addFieldMapping('field_key', 'unkey')->description('see prepareKey');
    $this->addFieldMapping('field_serial', 'well serial')
         ->description('See prepareKey().');
    $this->addFieldMapping('field_date', 'date');
    $this->addFieldMapping('field_time1', 'time');
    $this->addFieldMapping('field_datetime', 'datetime');
    $this->addFieldMapping('field_level', 'level');
    $this->addFieldMapping('field_units', 'units');
    $this->addFieldMapping('field_well_calibration', 'calibration');

  }

  protected function csvcolumns() {
    $columns[0] = array('date', 'Date');
    $columns[1] = array('time', 'Time');
    $columns[2] = array('level', 'level');
    $columns[3] = array('units', 'Units, in or cm');
    return $columns;
  }

  public function prepareRow($row) {
    //skip header rows
    //if(preg_match('/^([1-9]|0[1-9]|[12][0-9]|3[01])\D([1-9]|0[1-9]|1[012])\D(19[0-9][0-9]|20[0-9][0-9])$/' ,$row->date) === FALSE) {
    //  return FALSE;
    //}
    
    //Skip rows where level == --.-
    
    // convert to cm
    if($row->units == "in") {
      $row->level = $row->level * 2.54;
      $row->units = "cm";
    }
    
  }

  /**
   * Construct the machine name from the source file name.
   */
  protected function generateMachineName($class_name = NULL) {
    return drupal_strtolower(pathinfo($this->arguments['source_file'],
                                      PATHINFO_FILENAME));
  }

  public function fields() {
    return array(
      'serial' => 'The reference serial made during prepareKey().',
      'unkey' => 'The unique key',
      'level' => 'Level, manipulated during prepareRow().',
      'datetime' => 'The datetime, populated during prepareRow().',
    );
  }
  
  public function prepareKey($source_key, $row) {
  
    //Make row->serial
    $serial = ltrim(explode("_",pathinfo($this->arguments['source_file'],PATHINFO_FILENAME))[0],"0"); 
    $row->serial = $serial;
  
    // Turn date and time into datetime
    $row->datetime = strtotime($row->date . " " . $row->time);
    
    $key = array('unkey' => (string) $row->serial . "-" . (string) $row->datetime);
    $row->unkey = $key;
    return $key;
  }
}

