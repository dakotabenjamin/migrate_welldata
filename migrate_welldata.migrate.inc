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
          'datetime' => array('type' => 'varchar',
                           'length' => 10,
                           'not null' => TRUE,
                           'description' => 'date',
                          ),
          'serial' => array('type' => 'varchar',
                           'length' => 255,
                           'not null' => TRUE,
                           'description' => 'well serial',
                          ),
        ),
        MigrateDestinationNode::getKeySchema()
      );

    // Create a MigrateSource object, which manages retrieving the input data.
    $this->source = new MigrateSourceCSV($arguments['source_file'], $this->csvcolumns(), array(), $this->fields());

    $this->destination = new MigrateDestinationNode('migrate_welldata');

    $this->addFieldMapping('well', 'well')
         ->description('See prepareRow().');
    $this->addFieldMapping('field_date', 'date');
    $this->addFieldMapping('field_time1', 'time');
    $this->addFieldMapping('field_datetime', 'datetime');
    $this->addFieldMapping('field_level', 'level');
    $this->addFieldMapping('field_units', 'units');
    $this->addFieldMapping('field_well_calibration', 'calibration');
    $this->addFieldMapping('field_well_well', 'well');

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
    if(preg_match('/^([1-9]|0[1-9]|[12][0-9]|3[01])\D([1-9]|0[1-9]|1[012])\D(19[0-9][0-9]|20[0-9][0-9])$/' ,$row->date) === FALSE) {
      return FALSE;
    }
    
    //Skip rows where level == --.-
  
    //Make the serial >>>>>>>>>>>>> FIGURE OUT <<<<<<<<<<<<<<<<

    // Turn date and time into datetime
    $row->datetime = strtotime($row->date . " " . $row->time);
    
    // convert to cm
    if($row->units == "in") {
      $row->level = $row->level * 2.54;
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
//      'well' => 'The reference serial made during prepareRow().',
      'level' => 'Level, manipulated during prepareRow().',
      'datetime' => 'The datetime, populated during prepareRow().',
    );
  }
  
  public function prepareKey($source_key, $row) {
    $key = array();
    foreach ($source_key as $field_name => $field_schema) {
      $key[$field_name] = $row->$field_name;
    }
    return $key;
}
}
